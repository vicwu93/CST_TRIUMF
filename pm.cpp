
#include <zmq.hpp>
#include <iostream>
#include <queue>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <ctime>
#include "request_structure.h"


// Bens Helper function to do a printf, but automatically prepend a timestamp.
void ts_printf(const char *format, ...) {
  // Handle va args for message
  va_list argptr;
  char message[10000];
  va_start(argptr, format);
  vsprintf(message, (char*) format, argptr);
  va_end(argptr);

  // Handle timestamp
  time_t rawtime;
  timeval now;
  struct tm *timeinfo;
  char ts[80];

  time(&rawtime);
  gettimeofday(&now, NULL);

  int milli = now.tv_usec / 1000.;
  timeinfo = localtime(&rawtime);

  strftime(ts, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
  printf("%s.%03d %s", ts, milli, message);
}

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

    std::queue <RequestData> waiting;
    std::queue <RequestData> active;

    bool txReady = true;


    //  Process messages from both sockets
    while (1) {
        
        zmq::poll (&items [0], 1, 0);
        
        if (items[0].revents & ZMQ_POLLIN) {
            zmq_recv (receiver, buf, sizeof(buf), 0);
            if(strcmp(buf, "1") == 0){
                active.pop();
                txReady = true;
                std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "  << std::endl;
                ts_printf("- SIZE OF WAITING QUEUE %d\n",  waiting.size());
                ts_printf("- SIZE OF ACTIVE QUEUE %d\n",  active.size());
                ts_printf("- Rx received all tx data, pop queue \n");

            }else{
                request.decode(buf);
                std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "  << std::endl;
                ts_printf("- Received from rx: %s\n",  request.port);
                // std::cout << "Received from rx: " << request.port << std::endl;
                waiting.push(request);


                ts_printf("- SIZE OF WAITING QUEUE %d\n",  waiting.size());
                ts_printf("- SIZE OF ACTIVE QUEUE %d\n",  active.size());
                // std::cout << "SIZE OF WAITING QUEUE " << waiting.size() << std::endl;
                // std::cout << "SIZE OF ACTIVE QUEUE " << active.size() << std::endl;
                
            }
            
        }

        if(waiting.size() > 0 && active.size() < 2){

            active.push(waiting.front());
            waiting.pop();
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "<< std::endl;
            // ts_printf("- Rx received all tx data, pop queue \n");
            ts_printf("- SIZE OF WAITING QUEUE %d\n",  waiting.size());
            ts_printf("- SIZE OF ACTIVE QUEUE %d\n",  active.size());
        }

        if(active.size() == 2 && txReady){
            txReady = false;

            ts_printf("- Sending to tx nodes: %s\n",  active.front().port);
            active.front().encode(buf);
            zmq_send (publisher, buf, sizeof(buf), 0);
        }
        
    }
    return 0;
}