/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/BuiltinServiceProvider.hpp"

#include "srl/BuiltinMessageFactory.hpp"
#include "srl/Controller.hpp"

#include "srl/LoopbackConnection.hpp"
#include "srl/Log.hpp"

namespace al { namespace srl
{
    BuiltinServiceProvider::BuiltinServiceProvider( Controller & controller) :
        controller(controller),
        ServiceProvider("Builtin", NULL)
    {
        connection = new LoopbackConnection;
        controller.add_connection(connection);
        add_service("NoOp");
        add_service("Shutdown");
        add_service("RegisterService");
    }

    void BuiltinServiceProvider::handle_NoOp(
            const InterfaceMessage & message,
            Connection * const client,
            Log & log )
    {
        log.write(Log::Info, "[BuiltinServiceProvider] NoOp");
        connection->send(StatusMessageFactory::generate(message, "Did nothing, just like you asked."));
    }

    void BuiltinServiceProvider::handle_Shutdown(
            const InterfaceMessage & message,
            Connection * const client,
            Log & log )
    {
        log.write(Log::Info, "[BuiltinServiceProvider] Shutdown");
        connection->send(StatusMessageFactory::generate(message, "Server shutting down now..."));
        controller.stop();
    }

    void BuiltinServiceProvider::handle_RegisterService(
            const InterfaceMessage & message,
            Connection * const client,
            Log & log )
    {
        log.write(Log::Info, "[BuiltinServiceProvider] RegisterService");

        std::string provider_name;
        std::vector<std::string> services;
        if (RegisterServiceMessageFactory::parse(message, provider_name, services))
        {
            // Find the service provider, if available.
            ServiceProvider * provider = controller.get_provider(provider_name);

            // Make sure provider connection matches client.
            if (provider && (provider->get_connection() != client))
            {
                log.write(Log::Warning,
                        "[BuiltinServiceProvider]   - Provider %s being overriden with new client.",
                        provider_name.c_str());

                controller.unregister_provider(
                        provider_name, "New client registered services under this name.");
                provider = 0;
            }

            // Add new provider if necessary.
            if (!provider)
            {
                log.write(Log::Info, "[BuiltinServiceProvider]   - New provider %s being added.",
                        provider_name.c_str());
                provider = new ServiceProvider(provider_name, client);
                controller.register_provider(provider);
            }

            // Add services.
            for (int i = 0; i < services.size(); ++i)
            {
                provider->add_service(services[i]);
                log.write(Log::Info, "[BuiltinServiceProvider]     - Added service %s", services[i].c_str());
            }

            // Done.
            connection->send(StatusMessageFactory::generate(message, "Services added"));
        }
        else
        {
            log.write(Log::Error, "[BuiltinServiceProvider]   - Message parsing failed.");
            connection->send(ErrorMessageFactory::generate(message, "Parse error"));
        }
    }

    void BuiltinServiceProvider::handle_message(
            const InterfaceMessage & message,
            Connection * const client,
            Log & log )
    {
        if (get_name() == message.get_module())
        {
            std::string service = message.get_service();
                 if ("NoOp" == service) handle_NoOp(message, client, log);
            else if ("Shutdown" == service) handle_Shutdown(message, client, log);
            else if ("RegisterService" == service) handle_RegisterService(message, client, log);
            else connection->send(ErrorMessageFactory::generate(message, "Invalid service"));
        } else connection->send(ErrorMessageFactory::generate(message, "Invalid module"));
    }
} }
