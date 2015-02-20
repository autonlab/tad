/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/Controller.hpp"
#include "srl/BuiltinServiceProvider.hpp"
#include "concurrent/Time.hpp"

#include <iostream>
using namespace std;

namespace al { namespace srl
{
    void Controller::cleanup( void )
    {
        std::cout << "  - Closing routers..." << std::endl;
        for (int i = 0; i < routers.size(); ++i)
            routers[i]->stop();
        for (int i = 0; i < routers.size(); ++i)
        {
            routers[i]->join();
            delete routers[i];
        }
        std::cout << "  - Closing providers..." << std::endl;
        for (auto iter = providers.begin(); iter != providers.end(); ++iter)
        {
            ServiceProvider * provider = iter->second;
            if (provider->get_connection() && provider->get_connection()->is_connected())
                provider->get_connection()->send(ShutdownMessageFactory::generate());

            delete iter->second;
        }
        std::cout << "  - Closing managed comm interfaces..." << std::endl;
        for (int i = 0; i < comm_ifs.size(); ++i)
        {
            if (comm_ifs[i].is_managed())
            {
                if (comm_ifs[i].get_interface())
                    delete comm_ifs[i].get_interface();
            }
        }
        std::cout << "  - Closing connections..." << std::endl;
        ConnectionDescriptor * cd;
        while (active_connections.pop(cd));
        for (auto iter = connections.begin(); iter != connections.end(); ++iter)
        {
            iter->second.get_connection()->disconnect();
            delete iter->second.get_connection();
        }
    }

    void * Controller::run_loop( void )
    {
        // Add the builtin service provider.
        BuiltinServiceProvider * provider = new BuiltinServiceProvider(*this);
        register_provider(provider);

        // Start some routers.
        routers.resize(1);
        for (int i = 0; i < 1; ++i)
        {
            routers[i] = new Router(*this);
            routers[i]->start();
        }

        while (!stopped)
        {
            // Loop through connections looking for messages and removing disconnected
            // connections.
            long current_time = concurrent::stime();
            auto iter = connections.begin();
            while (iter != connections.end())
            {
                ConnectionDescriptor & cd = iter->second;
                Connection * connection = cd.get_connection();
                bool expired = cd.is_expired(current_time);

                // If connection was disconnected, destroy it.
                if (!connection->is_connected())
                {
                    cout << " - Connection appears disconnected. Deleting..." << endl;
                    destroy_connection(connection);
                    connections.erase(iter++);
                }

                // If the connection is passed its expiration, destroy it.
                else if (expired && !cd.is_being_processed())
                {
                    cout << " - Connection has expired. Deleting..." << endl;
                    destroy_connection(connection, "Session timed out.");
                    connections.erase(iter++);
                }
                else
                {
                    // Data is available?
                    if (connection->is_message_available() && !active_connections.is_in_queue(&cd))
                    {
                        cout << " - Connection has activity, pushing to active queue." << endl;
                        if (cd.get_expiration() > 0)
                            cd.set_expiration(concurrent::stime() + connection_timeout);
                        cd.set_processing(true);
                        active_connections.push(&cd);
                    }
                    ++iter;
                }
            }

            concurrent::msleep(10);
        }

        std::cout << "Stopping controller." << std::endl;
        cleanup();
    }
} }
