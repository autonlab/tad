#include "srl.hpp"
#include "srl/Message.hpp"
#include "srl/CallbackConnection.hpp"
#include "srl/CallbackInterface.hpp"
#include "srl/TCPInterface.hpp"
#include "srl/Controller.hpp"
#include "srl/BuiltinMessageFactory.hpp"

#include "network/Port.hpp"

#include "concurrent/Time.hpp"

#include <csignal>
#include <cstring>

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

namespace al { namespace platform {
    class SignalHandler
    {
        public:
            static inline bool register_signal( const int signal )
            {
                struct sigaction action;
                memset(&action, 0, sizeof(action));
                action.sa_handler = signal_handler;
                sigemptyset(&action.sa_mask);
                action.sa_flags = 0;
                return sigaction(signal, &action, 0) == 0;
            }

            static inline bool signal_was_received( void ) { return received_signal; }
            static inline int get_last_signal( void ) { return last_signal; }

        private:
            static void signal_handler( const int signal )
            {
                received_signal = true;
                last_signal = signal;
            }

        private:
            static volatile bool    received_signal;
            static volatile int     last_signal;
    };

    volatile bool SignalHandler::received_signal    = false;
    volatile int  SignalHandler::last_signal        = 0;
} }

int main( void )
{
    // Register signal handlers.
    platform::SignalHandler::register_signal(SIGINT);
    platform::SignalHandler::register_signal(SIGABRT);
    platform::SignalHandler::register_signal(SIGQUIT);

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
    while (controller.is_running() && !platform::SignalHandler::signal_was_received())
        concurrent::ssleep(1);

    // Stopped by signal?
    if (platform::SignalHandler::signal_was_received())
        cout << "*** Received signal " << strsignal(platform::SignalHandler::get_last_signal()) << endl;

    if (controller.is_running()) controller.stop();
    controller.join();

    // Done.
    cout << "Done." << endl;

    return 0;
}
