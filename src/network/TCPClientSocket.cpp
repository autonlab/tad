/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "network/TCPPort.hpp"

#include <cstring>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#include <sys/ioctl.h>

#include <iostream>
using namespace std;

namespace al { namespace network
{
    TCPClientSocket::TCPClientSocket( const std::string server, const std::string service )
        : address(0), socket_handle(-1)
    {
        // Build socket address.
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(
                    server.c_str(),
                    service == "" ? NULL : service.c_str(),
                    &hints,
                    &address) != 0)
            address = 0;
    }

    TCPClientSocket::~TCPClientSocket( void )
    {
        cout << "TCPClientSocket::~TCPClientSocket destroyed" << endl;
        disconnect();
        if (address)
            freeaddrinfo(address);
    }

    bool TCPClientSocket::connect( void )
    {
        cout << "TCPClientSocket::Connect called" << endl;

        // Disconnect if currently connected.
        disconnect();

        // Attempt to connect to an available address.
        addrinfo *current;
        int status = -1;
        for (current = address; current != NULL; current = current->ai_next)
        {
            // Attempt to open a socket handle.
            if ((socket_handle = socket(
                    current->ai_family,
                    current->ai_socktype,
                    current->ai_protocol)) == -1)
            {
                continue;
            }

            // Attempt connection.
            if ((status = ::connect(
                            socket_handle,
                            current->ai_addr,
                            current->ai_addrlen)) != -1)
            {
                break;
            }

            // Didn't work, disconnect socket.
            disconnect();
        }

        // If not connected, close socket.
        if (status == -1)
            disconnect();

        return status != -1;
    }

    void TCPClientSocket::disconnect( void )
    {
        cout << "TCPClientSocket::disconnect called" << endl;
        if (socket_handle != -1)
        {
            #ifdef WIN32
            closesocket(socket_handle);
            #else
            close(socket_handle);
            #endif
            socket_handle = -1;
        }
    }

    int TCPClientSocket::ready_to_read( const int timeout ) const
    {
        /*
        fd_set set;
        timeval wait;

        FD_ZERO(&set);
        FD_SET(socket_handle, &set);

        memset(&wait, 0, sizeof(wait));
        wait.tv_usec = timeout;
        
        int result = select(socket_handle + 1, &set, NULL, NULL, &wait);
        if (result == -1) return false; // Fail silently. ATW TODO: Log?
        else if (result == 0) return false; // Timeout; not ready.
        else return FD_ISSET(socket_handle, &set) != 0;
        */
        int bytes_available;
        ioctl(socket_handle, FIONREAD, &bytes_available);
        return bytes_available;
    }

    int TCPClientSocket::read( char buffer[], const int buffer_size )
    {
        return recv(socket_handle, buffer, buffer_size, 0);
    }
    
    bool TCPClientSocket::ready_to_write( const int timeout ) const
    {
        fd_set set;
        timeval wait;

        FD_ZERO(&set);
        FD_SET(socket_handle, &set);

        memset(&wait, 0, sizeof(wait));
        wait.tv_usec = timeout;
        
        int result = select(socket_handle + 1, NULL, &set, NULL, &wait);
        if (result == -1) return false; // Fail silently. ATW TODO: Log?
        else if (result == 0) return false; // Timeout; not ready.
        else return FD_ISSET(socket_handle, &set) != 0;
    }

    int TCPClientSocket::write( const char buffer[], const int buffer_size )
    {
        return send(socket_handle, buffer, buffer_size, 0);
    }
} }
