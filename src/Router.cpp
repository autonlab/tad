#include "srl/Router.hpp"
#include "srl/Controller.hpp"
#include "srl/Message.hpp"
#include "srl/BuiltinMessageFactory.hpp"
#include "concurrent/Time.hpp"

#include <iostream>
using namespace std;

namespace al { namespace srl
{
    void * Router::run_loop( void )
    {
        concurrent::Queue<Connection *> & active_connections =
            controller.get_active_connections();

        while (!stopped)
        {
            // Handle any active conections.
            Connection * connection;
            while (!stopped && active_connections.pop(connection))
            {
                // Get the message.
                std::string raw_message;
                while (!stopped && connection->receive(raw_message))
                {
                    // Parse message.
                    InterfaceMessage message;
                    message.decode(raw_message);
                    if (message.is_valid())
                    {
                        // Find the service provider and forward the message.
                        ServiceProvider * provider = controller.get_provider(message.get_module());
                        if (provider) provider->handle_message(message, connection);
                        else connection->send(ErrorMessageFactory::generate(
                                    message, "Unregistered provider"));
                    }
                }
            }

            concurrent::msleep(10);
        }

        cout << "Exiting router." << endl;
    }
} }
