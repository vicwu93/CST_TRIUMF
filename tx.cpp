
#include <zmq.hpp>
#include <iostream>
#include <queue>
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

    zmq::socket_t push3(context, ZMQ_PUSH);
    push3.connect("tcp://localhost:1238");

    zmq::pollitem_t items [] = {
        { static_cast<void*>(subscriber), 0, ZMQ_POLLIN, 0 }
    };


    char buf [BUFSIZE];
    RequestData request;

    DemoData data[3];

    uint64_t* buffer = new uint64_t (UINT64_MAX);

    DemoData data1;
    data1.channel_id = 1;
    data1.is_end_of_slice = false;
    data1.timestamp_secs = 11;

    DemoData data2;
    data2.channel_id = 2;
    data2.is_end_of_slice = false;
    data2.timestamp_secs = 22;

    DemoData data3;
    data3.channel_id = 3;
    data3.is_end_of_slice = true;
    data3.timestamp_secs = 33;

    data[0] = data1;
    data[1] = data2;
    data[2] = data3;


    std::queue <RequestData> sending;

    while(1){
        zmq::poll (&items [0], 1, 0);

        if (items[0].revents & ZMQ_POLLIN) {


            const auto len = zmq_recv (subscriber, buf, sizeof(buf), 0);
            request.decode(buf);
            sending.push(request);

            std::cout << "Received from manager: "<< request.port << std::endl;

            if(strcmp(request.port,"1236") == 0){
                
                sending.pop();

                for(int i = 0; i < 3; i++){
                    data[i].encode(buffer);
                    if(data[i].is_end_of_slice == true){ 
                        std::cout << "Sending to rx end of slice for data: " << data[i].channel_id <<std::endl;
                        zmq_send (push1, buffer, data[i].get_encoded_size_bytes(), 0);
                    }else{
                        std::cout << "Sending to rx for data: "<< data[i].channel_id <<std::endl;
                        zmq_send (push1, buffer, data[i].get_encoded_size_bytes(), 0);
                    }
                }
                

                

            }else if(strcmp(request.port,"1237") == 0){

                sending.pop();

                for(int i = 0; i < 3; i++){
                    data[i].encode(buffer);
                    if(data[i].is_end_of_slice == true){ 
                        std::cout << "Sending to rx end of slice for data: " << data[i].channel_id <<std::endl;
                        zmq_send (push2, buffer, data[i].get_encoded_size_bytes(), 0);
                    }else{
                        std::cout << "Sending to rx for data: "<< data[i].channel_id <<std::endl;
                        zmq_send (push2, buffer, data[i].get_encoded_size_bytes(), ZMQ_SNDMORE);
                    }
                }

            }else if(strcmp(request.port,"1238") == 0){

                sending.pop();

                for(int i = 0; i < 3; i++){
                    data[i].encode(buffer);
                    if(data[i].is_end_of_slice == true){ 
                        std::cout << "Sending to rx end of slice for data: " << data[i].channel_id <<std::endl;
                        zmq_send (push3, buffer, data[i].get_encoded_size_bytes(), 0);
                    }else{
                        std::cout << "Sending to rx for data: "<< data[i].channel_id <<std::endl;
                        zmq_send (push3, buffer, data[i].get_encoded_size_bytes(), ZMQ_SNDMORE);
                    }
                }
            }


        }


    }
    return 0;
}