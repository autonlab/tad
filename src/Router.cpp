/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/Router.hpp"
#include "srl/Controller.hpp"
#include "srl/BuiltinMessageFactory.hpp"
#include "concurrent/Time.hpp"

#include <iostream>
using namespace std;

namespace al { namespace srl
{
    void * Router::run_loop( void )
    {
        concurrent::UniqueQueue<Controller::ConnectionDescriptor *> & active_connections =
            controller.get_active_connections();

        while (!stopped)
        {
            // Handle any active conections.
            Controller::ConnectionDescriptor * cd;
            while (!stopped && active_connections.pop(cd, false))
            {
                cout << "    - Router: Found active connection. Handling..." << endl;

                // Get the message.
                Connection * connection = cd->get_connection();
                std::string raw_message;
                while (!stopped && connection->receive(raw_message))
                {
                    // Parse message.
                    InterfaceMessage message;
                    if (message.decode(raw_message))
                    {
                        // Is this going to a specific client?
                        if (message.get_client_id() > -1)
                        {
                            cout << "      . Found response to forward to client." << endl;
                            Controller::ConnectionDescriptor * dest_cd =
                                controller.get_connection(message.get_client_id());
                            if (dest_cd)
                            {
                                if (dest_cd->get_connection() && dest_cd->get_connection()->is_connected())
                                    dest_cd->get_connection()->send(raw_message);
                                else cout << "    * Err: Fwd to disconnected client" << endl;
                            } else cout << "    * Err: Fwd to unknown client" << endl;
                        }

                        // Find the service provider and forward the message.
                        else
                        {
                            cout << "      . Found request to forward to provider." << endl;
                            ServiceProvider * provider = controller.get_provider(message.get_module());
                            if (provider)
                            {
                                if (provider->is_service_available(message.get_service()))
                                {
                                    message.set_client_id(cd->get_id());
                                    provider->handle_message(message, connection);
                                }
                                else connection->send(ErrorMessageFactory::generate(
                                            message, "Invalid service"));
                            } else connection->send(ErrorMessageFactory::generate(
                                        message, "Unregistered provider"));
                        }
                    }
                    else
                    {
                        InterfaceMessage invalid_message("NA", "NA");
                        invalid_message["raw-message"] = raw_message;
                        connection->send(ErrorMessageFactory::generate(
                                invalid_message, message.get_last_error()));
                    }
                }

                // Release from queue.
                cout << "    - Router: Finished handling." << endl;
                cd->set_processing(false);
                active_connections.release(cd);
            }

            concurrent::msleep(10);
        }

        cout << "    - Exiting router." << endl;
    }
} }
