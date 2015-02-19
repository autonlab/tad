/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/TCPInterface.hpp"
#include "srl/TCPConnection.hpp"
#include "srl/Controller.hpp"
#include "concurrent/Time.hpp"

namespace al { namespace srl
{
    TCPInterface::TCPInterface( Controller & controller, const std::string service_name ) :
        CommIF(controller), monitor(controller, socket)
    {
        socket.connect(service_name);
        monitor.start();
    }

    TCPInterface::TCPInterface( Controller & controller, const int port ) :
        CommIF(controller), monitor(controller, socket)
    {
        socket.connect(port);
        monitor.start();
    }

    void * TCPInterface::Monitor::run_loop( void )
    {
        while (!stopped)
        {
            if (socket.is_connected())
            {
                network::TCPClientSocket * client = 0;
                while (socket.get_client_connection(client))
                {
                    controller.add_connection(new TCPConnection(client));
                    client = 0;
                }

                concurrent::msleep(100);
            }
            else concurrent::ssleep(1);
        }
    }
} }
