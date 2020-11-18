#include <zmq.hpp>
#include <iostream>
#include <unistd.h>
#include <string>
#include "request_structure.h"
#include "demo_data_structure.h"

// void encode(RequestData data, char *buf);

int main()
{
    int txCount = 0;
    int nTx = 2;
    // 2 is number of tx running
    bool rxFlags[2] = {0};
    int numAnalyzed = 0;

    zmq::context_t context(1);

    //  Manager node
    zmq::socket_t push(context, ZMQ_PUSH);
    push.connect("tcp://localhost:1234");

    // Pull socket that receives data from tx nodes
    zmq::socket_t pull(context, ZMQ_PULL);
    pull.bind("tcp://*:1236");

    zmq::pollitem_t items[] = {
        {static_cast<void *>(pull), 0, ZMQ_POLLIN, 0}};

    //Request data containing port number
    RequestData request;
    strcpy(request.port, "1236");
    char requestbuffer[sizeof(request)];
    request.encode(requestbuffer);

    // initial request outside of while loop
    std::cout << "Send request to manager: " << request.port << std::endl;
    zmq_send(push, requestbuffer, sizeof(requestbuffer), 0);

    uint64_t *buffer = new uint64_t(UINT64_MAX);
    DemoData data;

    // Message to send to pm comfirming all tx data is recieved
    char finishedbuff[] = {"1"};

    // int rcvcheck;
    // size_t rcvcheck_size = sizeof(rcvcheck);

    //  Process messages from both sockets
    while (true)
    {
        zmq::poll(&items[0], 1, 0);

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq_recv(pull, buffer, UINT64_MAX, 0);

            data.decode(buffer);
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~ "<< std::endl;
            std::cout << "Received from tx: " << data.channel_id << std::endl;
            // std::cout << "Is end of slice: " << data.is_end_of_slice << std::endl;

             
            if(data.is_end_of_slice){
                // std::cout << "End of slice received " << std::endl;
                // txCount++;
                std::cout << "Is end of slice: True" << std::endl;
                rxFlags[txCount] = true;
                txCount++;
            }else{
                std::cout << "Is end of slice: False" << std::endl;

                // std::cout << "End of slice received " << std::endl;

                // Analyze data?
                // numAnalyzed = ++numAnalyzed;
            }
            

            // if(zmq_getsockopt(pull, ZMQ_RCVMORE, &rcvcheck, &rcvcheck_size) == 0){
            //      std::cout << "ZMQ_RCVMORE " << data.is_end_of_slice << std::endl;
            // }else{
            //     std::cout << "else " << data.timestamp_secs << std::endl;
            //     std::cout << "Is end of slice: " << data.is_end_of_slice << std::endl;
            //     txCount++;
            //     std::cout << "Hello? " << txCount << std::endl;
            // }
        }

        if (std::all_of(
            std::begin(rxFlags), 
            std::end(rxFlags), 
            [](bool i)
            { 
              return i; // or return !i ;
            }
        )){
            for(int i = 0; i < nTx; i++) {
                rxFlags[i] = 0;
            }
            std::cout << "All tx data recevied" << std::endl;
            txCount = 0;
            zmq_send(push, finishedbuff, sizeof(finishedbuff), 0);
            sleep(2);
            std::cout << "Sending request to manager with port: " << request.port << std::endl;
            zmq_send(push, requestbuffer, sizeof(requestbuffer), 0);
        }
    }
    return 0;
}


