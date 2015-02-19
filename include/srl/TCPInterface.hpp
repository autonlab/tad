/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_TCP_Interface_hpp__
#define __SRL_TCP_Interface_hpp__

#include "srl/CommunicationInterface.hpp"
#include "concurrent/Thread.hpp"
#include "network/TCPPort.hpp"

namespace al { namespace srl
{
    /*!
     * This class implements a listener interface for TCP conections on the specified
     * port. This will create a new thread to watch for these connections.
     */
    class TCPInterface : public CommIF
    {
        public:
            /*!
             * @param controller The system controller.
             * @param service_name The service name to listen on.
             */
            TCPInterface( Controller & controller, const std::string service_name );

            /*!
             * @param controller The system controller.
             * @param port The port to listen on.
             */
            TCPInterface( Controller & controller, const int port );

            /*!
             * The system controller will just delete this interface so the destructor
             * will be used to stop the thread and disconnect the socket.
             */
            virtual ~TCPInterface( void )
            {
                monitor.stop();
                monitor.join();
                socket.disconnect();
            }

            /*!
             * @return True if bound to a port, false otherwise.
             */
            bool is_connected( void ) const { return socket.is_connected(); }

            /*!
             * Bind to the service.
             *
             * @param service_name The service to bind to.
             * @return True if sucessful, false otherwise.
             */
            bool connect( const std::string service_name ) { return socket.connect(service_name); }

            /*!
             * Bind to the port.
             *
             * @param port The port to bind to.
             * @return True if successful, false otherwise.
             */
            bool connect( const int port ) { return socket.connect(port); }

        private:
            class Monitor : public concurrent::Thread
            {
                public:
                    Monitor( Controller & controller, network::TCPServerSocket & socket ) :
                        controller(controller), socket(socket) { }

                protected:
                    virtual void * run_loop( void );

                private:
                    Controller & controller;
                    network::TCPServerSocket & socket;
            };

        private:
            Monitor monitor;
            network::TCPServerSocket socket;
    };
} }

#endif
