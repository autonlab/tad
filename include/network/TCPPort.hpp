/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __Network_TCP_Port_hpp__
#define __Network_TCP_Port_hpp__

#include "network/Port.hpp"

#include <sstream>

#ifdef WIN32
#include <winsock.h>
#else
#include <netdb.h>
#endif

namespace al { namespace network
{
    /*!
     * This class implements a generic TCP client socket.
     */
    class TCPClientSocket : public Port
    {
        public:
            static const int DefaultReadTimeout  = 1000; // 1ms.
            static const int DefaultWriteTimeout = 1000; // 1ms.

        public:
            /*!
             * @param socket_handle If a client socket is already opened, this constructor
             *          can be used to allow the class to handle the connection. The class will
             *          take care of disconnecting the socket.
             */
            TCPClientSocket( const int socket_handle ) : address(0), socket_handle(socket_handle) { }

            /*!
             * @param server The server host name (and, optionally, port) to connect to.
             * @param service The service name to connect to, in lieu of a port.
             */
            TCPClientSocket( const std::string server, const std::string service );

            virtual ~TCPClientSocket( void );

            /*!
             * @return True if the socket is connected, false otherwise.
             */
            inline bool is_connected( void ) const { return socket_handle != -1; }

            /*!
             * Connect the client socket (or reconnect if it was disconnected). This will not
             * work if the socket was initialized with a socket_handle. It will just disconnect.
             *
             * @return True if the connection was successful, false otherwise.
             */
            bool connect( void );

            /*!
             * Disconnects the socket.
             */
            void disconnect( void );

            /*!
             * Determine if there is data on the port.
             *
             * @param timeout The time (in microsecds) to wait for data to read.
             * @return The number of bytes available to be read.
             */
            int ready_to_read( const int timeout = DefaultReadTimeout ) const;

            /*!
             * @see Port::read
             */
            virtual int read( char buffer[], const int buffer_size );
            using Port::read;

            /*!
             * Determine if the port is ready to be written to.
             *
             * @param timeout The time (in microseconds) to wait for the port to be ready.
             * @return True if the port is ready to be written to, false otherwise.
             */
            bool ready_to_write( const int timeout = DefaultWriteTimeout ) const;

            /*!
             * @see Port::write
             */
            virtual int write( const char buffer[], const int buffer_size );
            using Port::write;

        private:
            addrinfo *address;
            int socket_handle;
    };

    /*!
     * This class implements a TCP server socket. It is not a port that can be read from or
     * written to. Instead, it listens on a specific port and allows easy handling of new
     * connections.
     */
    class TCPServerSocket
    {
        public:
            static const int DefaultWaitTimeout = 1000; // 1ms

        public:
            TCPServerSocket( void ) : socket_handle(-1) { }
            ~TCPServerSocket( void ) { if (socket_handle) disconnect(); }
            
            /*!
             * @return True if connected, false otherwise. */
            bool is_connected( void ) const { return socket_handle != -1; }

            /*!
             * Binds to the specified port.
             *
             * @param service_name The service name to bind to.
             * @return True if successful, false otherwise.
             */
            bool connect( const std::string service_name );

            /*!
             * Binds to the specified port.
             *
             * @param port The port number in host byte order.
             * @return True if successful, false otherwise.
             */
            bool connect( const int port );

            /*!
             * Unbinds the listening socket.
             */
            void disconnect( void );

            /*!
             * Get the next waiting client connection.
             *
             * @param[out] client_socket The socket that will be updated.
             * @param The time to wait for a new client.
             * @return True if a client was connected, false otherwise.
             * @note If The client_socket parameter is not NULL, this function will
             *          disconnect the client first. So don't send in an uninitialized
             *          variable!
             */
            bool get_client_connection(
                    TCPClientSocket * & client_socket,
                    const int timeout = DefaultWaitTimeout );

        private:
            int socket_handle;
    };
} }

#endif
