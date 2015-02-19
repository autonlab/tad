#include "srl.hpp"
#include "srl/Message.hpp"
#include "srl/CallbackConnection.hpp"
#include "srl/CallbackInterface.hpp"
#include "srl/TCPInterface.hpp"
#include "srl/Controller.hpp"
#include "srl/BuiltinMessageFactory.hpp"

#include "network/Port.hpp"

#include "concurrent/Time.hpp"

#include <iostream>

using namespace std;
using namespace al;

class MyService : public srl::CallbackConnection
{
    protected:
        virtual void handle( const std::string raw_message )
        {
            srl::InterfaceMessage message;
            message.decode(raw_message);
            if (message.get_module() == "MyService")
            {
                if (message.get_service() == "serviceA")
                {
                    if (message["a"].is_integer() && message["b"].is_integer())
                    {
                        srl::InterfaceMessage response(message.get_module(), message.get_service());
                        response["result"] = message["a"].integer() - message["b"].integer();
                        response["original-message"] = static_cast<srl::Field>(message.get_wrapper());
                        send(response.encode());
                    } else send(srl::ErrorMessageFactory::generate(message, "Integers a and b must be provided."));
                } else send(srl::ErrorMessageFactory::generate(message, "Service not implemented yet.")); 
            }
        }

};

int main( void )
{
    srl::Controller controller;

    // Establish available communication interfaces. Make them unmanaged.
    srl::CallbackInterface callback_interface(controller);
    controller.register_interface(&callback_interface, false);

    // Create an interface for handling TCP connections.
    controller.register_interface(new srl::TCPInterface(controller, 12345));

    // Start the controller.
    controller.start();

    // Create a service connection.
    MyService service_connection;
    callback_interface.connect(service_connection);

    // Connect a new service.
    service_connection.send(srl::RegisterServiceMessageFactory::generate(
                "MyService", std::vector<string>({"serviceA", "serviceB"})));

    concurrent::msleep(100);

    // Wait for server to finish.
    cout << "Waiting for server to close..." << endl;
    controller.join();

    // Done.
    cout << "Done." << endl;

    return 0;
}
