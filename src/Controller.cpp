/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/Controller.hpp"
#include "srl/BuiltinServiceProvider.hpp"
#include "concurrent/Time.hpp"

namespace al { namespace srl
{
    const std::string Controller::log_path  = "logs/";
    const bool Controller::logging          = true;

    void Controller::cleanup( void )
    {
        log.write(Log::Info, "[Controller::Cleanup] - Closing routers.");
        for (int i = 0; i < routers.size(); ++i)
            routers[i]->stop();
        for (int i = 0; i < routers.size(); ++i)
        {
            routers[i]->join();
            delete routers[i];
        }

        log.write(Log::Info, "[Controller::Cleanup] - Closing providers.");
        for (auto iter = providers.begin(); iter != providers.end(); ++iter)
        {
            ServiceProvider * provider = iter->second;
            if (provider->get_connection() && provider->get_connection()->is_connected())
                provider->get_connection()->send(ShutdownMessageFactory::generate());

            delete iter->second;
        }

        log.write(Log::Info, "[Controller::Cleanup] - Closing managed communication interfaces.");
        for (int i = 0; i < comm_ifs.size(); ++i)
        {
            if (comm_ifs[i].is_managed())
            {
                if (comm_ifs[i].get_interface())
                    delete comm_ifs[i].get_interface();
            }
        }

        log.write(Log::Info, "[Controller::Cleanup] - Closing connections.");
        ConnectionDescriptor * cd;
        while (active_connections.pop(cd));
        for (auto iter = connections.begin(); iter != connections.end(); ++iter)
        {
            iter->second.get_connection()->disconnect();
            delete iter->second.get_connection();
        }

        log.write(Log::Info, "[Controller::Cleanup] Finished.");
    }

    void * Controller::run_loop( void )
    {
        if (is_logging())
        {
            log.append_to_file(get_log_path() + "controller.log");
            log.append_to_handle(stdout, Log::NotErrors);
            log.append_to_handle(stderr, {Log::Error});
        }

        log.write(Log::Info, "[Controller] Started.");

        // Add the builtin service provider.
        log.write(Log::Info, "[Controller]   - Adding builtin services.");
        BuiltinServiceProvider * provider = new BuiltinServiceProvider(*this);
        register_provider(provider);

        // Start some routers.
        log.write(Log::Info, "[Controller]   - Starting %d routers.", router_count);
        routers.resize(router_count);
        for (int i = 0; i < router_count; ++i)
        {
            routers[i] = new Router(*this, i);
            routers[i]->start();
        }

        log.write(Log::Info, "[Controller] Starting loop.");
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
                    log.write(Log::Info, "[Controller]   - Found disconnected client. Deleting.");
                    destroy_connection(connection);
                    connections.erase(iter++);
                }

                // If the connection is passed its expiration, destroy it.
                else if (expired && !cd.is_being_processed())
                {
                    log.write(Log::Info, "[Controller]   - Found expired client. Deleting.");
                    destroy_connection(connection, "Session timed out.");
                    connections.erase(iter++);
                }
                else
                {
                    // Data is available?
                    if (connection->is_message_available() && !active_connections.is_in_queue(&cd))
                    {
                        log.write(Log::Info, "[Controller]   - Found active connection, moving to queue.");
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

        log.write(Log::Info, "[Controller] Exiting loop.");

        log.write(Log::Info, "[Controller] Calling cleanup.");
        cleanup();
    }
} }
