#include "srl.hpp"
#include "srl/Message.hpp"
#include "srl/CallbackConnection.hpp"
#include "srl/CallbackInterface.hpp"
#include "srl/TCPInterface.hpp"
#include "srl/Controller.hpp"
#include "srl/BuiltinMessageFactory.hpp"
#include "srl/Log.hpp"

#include "network/Port.hpp"

#include "concurrent/Time.hpp"

#include <csignal>
#include <cstring>

#include <dirent.h>
#include <sys/stat.h>

#include <iostream>

using namespace std;
using namespace al;
using srl::Log;

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

inline int directory_exists( const char * const name )
{
    DIR *dir    = NULL;
    int exists  = 0;

    if ((dir = opendir(name)) != NULL)
    {
        exists = 1;
        closedir(dir);
    }

    return exists;
}

inline int create_directory( const char * const name, const int permissions )
{
    return !(mkdir(name, permissions) < 0);
}

int main( void )
{
    if (!directory_exists("logs")) create_directory("logs", 0777);

    srl::Log log;
    log.append_to_file("logs/server.log");
    log.append_to_handle(stdout, {Log::Info, Log::Warning, Log::Debug});
    log.append_to_handle(stderr, {Log::Error});

    log.write(Log::Info, "Starting up server.");

    // Register signal handlers.
    log.write(Log::Info, "  - Registering signal handles.");
    platform::SignalHandler::register_signal(SIGINT);
    platform::SignalHandler::register_signal(SIGABRT);
    platform::SignalHandler::register_signal(SIGQUIT);

    srl::Controller controller;

    // Establish available communication interfaces. Make them unmanaged.
    log.write(Log::Info, "  - Registering callback interface.");
    srl::CallbackInterface callback_interface(controller);
    controller.register_interface(&callback_interface, false);

    // Create an interface for handling TCP connections.
    log.write(Log::Info, "  - Registering TCP interface.");
    controller.register_interface(new srl::TCPInterface(controller, 12345));

    // Start the controller.
    log.write(Log::Info, "  - Starting controller.");
    controller.start();

    concurrent::msleep(100);

    // Wait for server to finish.
    log.write(Log::Info, "  - Waiting for server to close.");
    while (controller.is_running() && !platform::SignalHandler::signal_was_received())
        concurrent::ssleep(1);

    // Stopped by signal?
    if (platform::SignalHandler::signal_was_received())
        log.write(Log::Info, "  - Received signal %s.", strsignal(platform::SignalHandler::get_last_signal()));

    log.write(Log::Info, "  - Stopping controller.");
    if (controller.is_running()) controller.stop();
    controller.join();

    // Done.
    log.write(Log::Info, "Server closed.");

    return 0;
}
