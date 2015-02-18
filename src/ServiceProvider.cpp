#include "srl/ServiceProvider.hpp"

#include "srl/Message.hpp"
#include "srl/BuiltinMessageFactory.hpp"

#include "srl/Controller.hpp"

#include <iostream>
using namespace std;

namespace al { namespace srl
{
    void BuiltinServiceProvider::handle_RegisterService( const InterfaceMessage & message, Connection * client )
    {
        string provider_name;
        vector<string> services;
        if (RegisterServiceMessageFactory::parse(message, provider_name, services))
        {
            // Find the service provider, if available.
            ServiceProvider * provider = controller.get_provider(provider_name);
            if (!provider)
            {
                cout << "Provider '" << provider_name << "' not found, adding new." << endl;
                provider = new ServiceProvider(provider_name, client);
                controller.add_provider(provider);
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
            if (service == "RegisterService") handle_RegisterService(message, client);
            else client->send(ErrorMessageFactory::generate(message, "Invalid service"));
        } else client->send(ErrorMessageFactory::generate(message, "Invalid module"));
    }
} }
