
#include <zmq.hpp>
#include <iostream>
#include "request_structure.h"


int main ()
{
    zmq::context_t context(1);

    //  Rx node
    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.bind("tcp://*:1234");

    //  Publisher to the Tx Nodes
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:1235");


    //  Initialize poll set
    zmq::pollitem_t items [] = {
        { static_cast<void*>(receiver), 0, ZMQ_POLLIN, 0 }
    };

    RequestData request;
    char buf [BUFSIZE];

    //  Process messages from both sockets
    while (1) {
        
        zmq::poll (&items [0], 1, 0);
        
        if (items[0].revents & ZMQ_POLLIN) {
            const auto len = zmq_recv (receiver, buf, sizeof(buf), 0);
            request.decode(buf);
            std::cout << "Received from rx: " << request.port << std::endl;

            std::cout << "Sending to tx nodes: " << request.port << std::endl;
            zmq_send (publisher, buf, sizeof(buf), 0);
        }
        
    }
    return 0;
}