//
//  Reading from multiple sockets in C++
//  This version uses zmq_poll()
//

#include "zhelpers.hpp"


int main ()
{
    zmq::context_t context(1);

    //  Connect to task ventilator
    zmq::socket_t push(context, ZMQ_PUSH);
    push.connect("tcp://localhost:1234");

    zmq::socket_t pull(context, ZMQ_PULL);
    pull.bind("tcp://*:1236");

    zmq::pollitem_t items [] = {{ 
        static_cast<void*>(pull), 0, ZMQ_POLLIN, 0 
    }};

    std::cout << "Send to pm"<< std::endl;
    zmq::message_t message;
    push.send(message);

    //  Process messages from both sockets
    while (1) {
        zmq::poll (&items[0], 1, 0);

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t update;
            pull.recv (&update);
            std::cout << "Received from tx"<< std::endl;
            // std::cout << update << std::endl;
            std::string updateMsg = std::string(static_cast<char*>(update.data()), update.size());
            std::cout << updateMsg << std::endl;

            std::cout << "Send to pm"<< std::endl;
            zmq::message_t message;
            push.send(message);
        }
    }
    return 0;
}
