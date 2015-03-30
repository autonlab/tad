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
        CommIF(controller), monitor(controller, socket, log)
    {
        if (controller.is_logging())
        {
            log.append_to_file(controller.get_log_path() + "if_tcp_" + service_name + ".log");
            log.append_to_handle(stdout, Log::NotErrors);
            log.append_to_handle(stderr, {Log::Error});
        }

        log.write(Log::Info, "[TCPInterface] Opening TCP interface on service %s", service_name.c_str());

        socket.connect(service_name);
        monitor.start();
    }

    TCPInterface::TCPInterface( Controller & controller, const int port ) :
        CommIF(controller), monitor(controller, socket, log)
    {
        if (controller.is_logging())
        {
            char buffer[50];
            snprintf(buffer, 50, "if_tcp_port_%d.log", port);

            log.append_to_file(controller.get_log_path() + buffer);
            log.append_to_handle(stdout, Log::NotErrors);
            log.append_to_handle(stderr, {Log::Error});
        }

        log.write(Log::Info, "[TCPInterface] Opening TCP interface on port %d", port);

        socket.connect(port);
        monitor.start();
    }

    void * TCPInterface::Monitor::run_loop( void )
    {
        log.write(Log::Info, "[TCPInterface] Entering loop.");
        while (!stopped)
        {
            if (socket.is_connected())
            {
                network::TCPClientSocket * client = 0;
                while (socket.get_client_connection(client))
                {
                    log.write(Log::Info, "[TCPInterface]   - Adding connection");
                    controller.add_connection(new TCPConnection(client));
                    client = 0;
                }

                concurrent::msleep(100);
            }
            else concurrent::ssleep(1);
        }
        log.write(Log::Info, "[TCPInterface] Exiting loop.");
    }
} }
