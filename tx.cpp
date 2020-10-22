//
//  Reading from multiple sockets in C++
//  This version uses zmq_poll()
//

#include "zhelpers.hpp"


int main ()
{

    zmq::context_t context(1);
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:1235");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::socket_t push(context, ZMQ_PUSH);
    push.connect("tcp://localhost:1236");

    zmq::pollitem_t items [] = {{ 
        static_cast<void*>(subscriber), 0, ZMQ_POLLIN, 0 
    }};

    while(1) {
        

        zmq::poll (&items [0], 1, 0);

        if (items[0].revents & ZMQ_POLLIN) {
            char arr[20] = {0};
            int rnd = rand() % 100;
            
            zmq::message_t update;
            std::cout << "Received from pm"<< std::endl;
            subscriber.recv (&update);

            // zmq::message_t message(100);
            // push.send(message);
            std::string data = "data" + std::to_string(rnd);
            zmq_send(push, data.c_str(), data.length(), 0);

            std::cout << "Sending data to rx for analysis"<< std::endl;
        }

    }
    return 0;
}