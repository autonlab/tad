/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/CallbackInterface.hpp"
#include "srl/Controller.hpp"

namespace al { namespace srl
{
    CallbackInterface::CallbackInterface( Controller & controller ) : CommIF(controller)
    {
        if (controller.is_logging())
        {
            log.append_to_file(controller.get_log_path() + "if_callback.log");
            log.append_to_handle(stdout, Log::NotErrors);
            log.append_to_handle(stderr, {Log::Error});
        }

        log.write(Log::Info, "[CallbackInterface] Interface opened.");
    }

    void CallbackInterface::connect( CallbackConnection & endpoint )
    {
        log.write(Log::Info, "[CallbackInterface] New connection added.");

        // Create a connection and connect to the endpoint.
        CallbackConnection * other_endpoint = new CallbackConnection;
        endpoint.connect(other_endpoint);
        other_endpoint->connect(&endpoint);

        // Add connection to controller with an expiration of -1, so the connection
        // won't expire.
        controller.add_connection(other_endpoint, -1);
    }
} }
