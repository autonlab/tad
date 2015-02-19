/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/BuiltinServiceProvider.hpp"

#include "srl/BuiltinMessageFactory.hpp"
#include "srl/Controller.hpp"

namespace al { namespace srl
{
    void BuiltinServiceProvider::handle_Shutdown( void ) { controller.stop(); }

    void BuiltinServiceProvider::handle_RegisterService( const InterfaceMessage & message, Connection * client )
    {
        std::string provider_name;
        std::vector<std::string> services;
        if (RegisterServiceMessageFactory::parse(message, provider_name, services))
        {
            // Find the service provider, if available.
            ServiceProvider * provider = controller.get_provider(provider_name);
            if (!provider)
            {
                provider = new ServiceProvider(provider_name, client);
                controller.register_provider(provider);
            }

            // Add services.
            for (int i = 0; i < services.size(); ++i)
                provider->add_service(services[i]);

            // Done.
            client->send(StatusMessageFactory::generate(message, "Services added"));
        } else client->send(ErrorMessageFactory::generate(message, "Parse error"));
    }

    void BuiltinServiceProvider::handle_message( const InterfaceMessage & message, Connection * client )
    {
        if (get_name() == message.get_module())
        {
            std::string service = message.get_service();
                 if ("Shutdown" == service) handle_Shutdown();
            else if ("RegisterService" == service) handle_RegisterService(message, client);
            else client->send(ErrorMessageFactory::generate(message, "Invalid service"));
        } else client->send(ErrorMessageFactory::generate(message, "Invalid module"));
    }
} }
