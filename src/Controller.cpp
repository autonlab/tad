#include "srl/Controller.hpp"
#include "concurrent/Time.hpp"

#include <iostream>

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
            delete iter->second;
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
        Connection * connection;
        while (active_connections.pop(connection));
        for (auto iter = connections.begin(); iter != connections.end(); ++iter)
        {
            (*iter)->disconnect();
            delete *iter;
        }
    }

    void * Controller::run_loop( void )
    {
        // Add the builtin service provider.
        BuiltinServiceProvider * provider = new BuiltinServiceProvider(*this);
        add_provider(provider);

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
            auto iter = connections.begin();
            while (iter != connections.end())
            {
                Connection * connection = *iter;

                // If connection was disconnected, destroy it.
                if (!connection->is_connected())
                {
                    delete connection;
                    connections.erase(iter++);
                }
                else
                {
                    // Data is available?
                    if (connection->is_message_available())
                        active_connections.push(connection);
                    ++iter;
                }
            }


            concurrent::msleep(200);
            static int i = 0;
            if (++i == 10) break;
        }

        std::cout << "Stopping controller." << std::endl;
        cleanup();
    }
} }
