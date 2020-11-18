#include <zmq.hpp>
#include <iostream>
#include <unistd.h>
#include <string>
#include "request_structure.h"
#include "demo_data_structure.h"

// void encode(RequestData data, char *buf);

int main()
{
    int nTx = 2;
    bool rxFlags[2] = {false};
    int txCount = 0;

    zmq::context_t context(1);

    //  Manager node
    zmq::socket_t push(context, ZMQ_PUSH);
    push.connect("tcp://localhost:1234");
    // Multiple pushes to pull

    zmq::socket_t pull(context, ZMQ_PULL);
    const char *port = "1238";

    pull.bind("tcp://*:1238");

    zmq::pollitem_t items[] = {
        {static_cast<void *>(pull), 0, ZMQ_POLLIN, 0}};

    RequestData request;
    // request.test = 21;
    strcpy(request.port, "1238");
    char requestbuffer[sizeof(request)];
    request.encode(requestbuffer);

    std::cout << "Send request to manager: " << request.port << std::endl;
    zmq_send(push, requestbuffer, sizeof(requestbuffer), 0);

    uint64_t *buffer = new uint64_t(UINT64_MAX);
    DemoData data;

    char finishedbuff[] = {"1"};

    //  Process messages from both sockets
    while (1)
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
                std::cout << "Is end of slice: True" << std::endl;
                rxFlags[txCount] = true;
                txCount++;
            } else {
                std::cout << "Is end of slice: False" << std::endl;
            }

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


