//
//  Reading from multiple sockets in C++
//  This version uses zmq_poll()
//

#include "zhelpers.hpp"


int main ()
{
    zmq::context_t context(1);

    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.bind("tcp://*:1234");

    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:1235");


    //  Initialize poll set
    zmq::pollitem_t items [] = {{ 
            static_cast<void*>(receiver), 0, ZMQ_POLLIN, 0 
        }
    };
    //  Process messages from both sockets
    while (1) {
        
        zmq::poll (&items [0], 1, 0);
        
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t message (5);
            receiver.recv(&message);
            std::cout << "Received request from rx " << std::endl;

            std::cout << "Send to tx" << std::endl;
            zmq::message_t request (12);
            memcpy (request.data (), "Pub-1 Data", 12);
            publisher.send (request);
        }
        
        sleep(1);
    }
    return 0;
}
