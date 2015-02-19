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
#include <fcntl.h>
#endif

#include <iostream>
using namespace std;

namespace al { namespace network
{
    bool TCPServerSocket::connect( const std::string service_name )
    {
        servent * service = getservbyname(service_name.c_str(), "tcp");
        if (service) return connect(ntohs(service->s_port));
        else return false;
    }

    bool TCPServerSocket::connect( const int port )
    {
        sockaddr_in server_address;

        // Create a non-blocking TCP socket.
        if ((socket_handle = socket(PF_INET, SOCK_STREAM, 0)) < 0) return false;
        fcntl(socket_handle, F_SETFL, O_NONBLOCK);

        // Allow socket reuse.
        int optval = 1;
        setsockopt(socket_handle, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        // Bind to port.
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family       = AF_INET;
        server_address.sin_addr.s_addr  = htonl(INADDR_ANY);
        server_address.sin_port         = htons(port);

        if (bind(socket_handle,
                    reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address)) >= 0)
            if (listen(socket_handle, 10) >= 0) return true;

        // Bind or listen failed. Close socket.
        disconnect();
        return false;
    }

    void TCPServerSocket::disconnect( void )
    {
        if (socket_handle > -1)
        {
            close(socket_handle);
            socket_handle = -1;
        }
    }

    bool TCPServerSocket::get_client_connection( TCPClientSocket * & client_socket, const int timeout )
    {
        // If client socket is already connected, disconnect and free socket.
        if (client_socket)
        {
            cout << "TCPServerSocket::deleting client socket" << endl;
            delete client_socket;
            client_socket = 0;
        }

        // 1ms timeout.
        timeval timeout_str;
        timeout_str.tv_sec  = timeout / 1000000;
        timeout_str.tv_usec = timeout % 1000000;

        // Only listening to server socket.
        fd_set socket_set;
        FD_ZERO(&socket_set);
        FD_SET(socket_handle, &socket_set);

        if (select(socket_handle + 1, &socket_set, NULL, NULL, &timeout_str) > 0)
        {
            sockaddr_in client_address;
            unsigned int client_address_size = sizeof(client_address);
            int client_socket_handle = accept(
                    socket_handle, reinterpret_cast<sockaddr *>(&client_address),
                    &client_address_size);
            if (client_socket_handle > 0)
                client_socket = new TCPClientSocket(client_socket_handle);
        }

        return client_socket != 0;
    }
} }
