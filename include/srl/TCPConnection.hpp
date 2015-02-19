/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_TCP_Connection_hpp__
#define __SRL_TCP_Connection_hpp__

#include "srl/Connection.hpp"
#include "network/TCPPort.hpp"

namespace al { namespace srl
{
    /*!
     * This class handles a TCP client connection. It manages a connected TCPClientSocket
     * instance.
     *
     * For undocumented methods in this class, @see Connection.
     */
    class TCPConnection : public Connection
    {
        public:
            /*!
             * @param socket The connected client socket to be managed. It will be destroyed
             *          by this class so it should not be destroyed elsewhere.
             */
            TCPConnection( network::TCPClientSocket * const socket ) : socket(socket) { }

            virtual ~TCPConnection( void )
            {
                if (socket)
                {
                    disconnect();
                    delete socket;
                    socket = 0;
                }
            }

            virtual bool send( const std::string message )
            {
                if (socket) return socket->write(message) == message.size();
                else return false;
            }

            virtual bool receive( std::string & message )
            {
                if (socket)
                {
                    int bytes_available = socket->ready_to_read();
                    if (bytes_available > 0) return socket->read(message) == bytes_available;
                }

                return false;
            }

            virtual bool is_message_available( void ) { return socket && (socket->ready_to_read() > 0);}

            virtual bool is_connected( void ) { return socket && socket->is_connected(); }
            virtual void disconnect( void ) { if (socket) socket->disconnect(); }

        private:
            network::TCPClientSocket * socket;
    };
} }

#endif
