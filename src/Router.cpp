/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/Router.hpp"
#include "srl/Controller.hpp"
#include "srl/BuiltinMessageFactory.hpp"
#include "concurrent/Time.hpp"

namespace al { namespace srl
{
    void * Router::run_loop( void )
    {
        if (controller.is_logging())
        {
            char buffer[20];
            snprintf(buffer, 20, "router_%d.log", id);

            log.append_to_file(controller.get_log_path() + buffer);
            log.append_to_handle(stdout, Log::NotErrors);
            log.append_to_handle(stderr, {Log::Error});
        }

        log.write(Log::Info, "[Router %d] Started", id);

        concurrent::UniqueQueue<Controller::ConnectionDescriptor *> & active_connections =
            controller.get_active_connections();

        log.write(Log::Info, "[Router %d] Entering loop.", id);
        while (!stopped)
        {
            // Handle any active conections.
            Controller::ConnectionDescriptor * cd;
            while (!stopped && active_connections.pop(cd, false))
            {
                log.write(Log::Info, "[Router %d]   - Handling connection.", id);

                // Get the message.
                Connection * connection = cd->get_connection();
                std::string raw_message;
                while (!stopped && connection->receive(raw_message))
                {
                    std::vector<std::string> blocks = extract_message_blocks(raw_message);
                    for (int i = 0; i < blocks.size(); ++i)
                    {
                        // Parse messages.
                        InterfaceMessage message;
                        if (message.decode(blocks[i]))
                        {
                            // Is this going to a specific client?
                            if (message.get_client_id() > -1)
                            {
                                log.write(
                                        Log::Info,
                                        "[Router %d]     - Response (%s, %s): Forwarding to client <%d>.",
                                        id,
                                        message.get_module().c_str(),
                                        message.get_service().c_str(),
                                        message.get_client_id());

                                Controller::ConnectionDescriptor * dest_cd =
                                    controller.get_connection(message.get_client_id());
                                if (dest_cd)
                                {
                                    if (dest_cd->get_connection() && dest_cd->get_connection()->is_connected())
                                        dest_cd->get_connection()->send(raw_message);
                                    else log.write(Log::Error,
                                            "[Router %d]     - Client seems disconnected.", id);
                                } else log.write(Log::Error,
                                        "[Router %d]     - Client does not exist.", id);
                            }

                            // Find the service provider and forward the message.
                            else
                            {
                                log.write(
                                        Log::Info,
                                        "[Router %d]     - Request (%s, %s): Forwarding to provider.",
                                        id,
                                        message.get_module().c_str(),
                                        message.get_service().c_str());

                                ServiceProvider * provider = controller.get_provider(message.get_module());
                                if (provider)
                                {
                                    if (provider->is_service_available(message.get_service()))
                                    {
                                        message.set_client_id(cd->get_id());
                                        provider->handle_message(message, connection, log);
                                    }
                                    else
                                    {
                                        connection->send(ErrorMessageFactory::generate(
                                                    message, "Invalid service"));
                                        log.write(Log::Error, "[Router %d]     - Invalid service request.", id);
                                    }
                                }
                                else
                                {
                                    connection->send(ErrorMessageFactory::generate(
                                                message, "Unregistered provider"));
                                    log.write(Log::Error, "[Router %d]     - Invalid module.", id);
                                }
                            }
                        }
                        else
                        {
                            InterfaceMessage invalid_message("NA", "NA");
                            invalid_message["raw-message"] = raw_message;
                            connection->send(ErrorMessageFactory::generate(
                                        invalid_message, message.get_last_error()));
                            log.write(Log::Error, "[Router %d]     - Message parse error.", id);
                        }
                    }
                }

                // Release from queue.
                log.write(Log::Info, "[Router %d]   - Finished with request.", id);
                cd->set_processing(false);
                active_connections.release(cd);
            }

            concurrent::msleep(10);
        }

        log.write(Log::Info, "[Router %d] Exiting.", id);
    }
} }
