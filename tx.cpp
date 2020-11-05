
#include <zmq.hpp>
#include <iostream>
#include "demo_data_structure.h"
#include "request_structure.h"


int main ()
{

    zmq::context_t context(1);
    // Subscriber to pub/Manager
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:1235");
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "", 0);

    // Push to Rx node
    zmq::socket_t push1(context, ZMQ_PUSH);
    push1.connect("tcp://localhost:1236");

    zmq::socket_t push2(context, ZMQ_PUSH);
    push2.connect("tcp://localhost:1237");

    zmq::pollitem_t items [] = {
        { static_cast<void*>(subscriber), 0, ZMQ_POLLIN, 0 }
    };


    char buf [BUFSIZE];
    RequestData request;

    DemoData data;
    data.channel_id = 7;
    uint64_t* buffer = new uint64_t (UINT64_MAX);
    data.encode(buffer);

    while(1){
        zmq::poll (&items [0], 1, 0);

        if (items[0].revents & ZMQ_POLLIN) {

            const auto len = zmq_recv (subscriber, buf, sizeof(buf), 0);
            request.decode(buf);
            std::cout << "Received from manager: "<< request.port << std::endl;

            if(strcmp(request.port,"1236") == 0){

                std::cout << "Sending to rx "<< buffer<<std::endl;
                zmq_send (push1, buffer, data.get_encoded_size_bytes(), 0);

            }else if(strcmp(request.port,"1237") == 0){

                std::cout << "Sending to rx "<< buffer<<std::endl;
                zmq_send (push2, buffer, data.get_encoded_size_bytes(), 0);

            }


            // switch(buf){
            //     case "1236" :
            //         std::cout << "Sending to rx"<< std::endl;
            //         zmq_send (push1, &buf, len, 0);
            //         break;
            //     case "1237" :
            //         std::cout << "Sending to rx"<< std::endl;
            //         zmq_send (push2, &buf, len, 0);
            //         break;
            // }

        }


    }
    return 0;
}