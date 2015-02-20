/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/CallbackInterface.hpp"
#include "srl/Controller.hpp"

namespace al { namespace srl
{
    CallbackInterface::CallbackInterface( Controller & controller ) : CommIF(controller) { }

    void CallbackInterface::connect( CallbackConnection & endpoint )
    {
        // Create a connection and connect to the endpoint.
        CallbackConnection * other_endpoint = new CallbackConnection;
        endpoint.connect(other_endpoint);
        other_endpoint->connect(&endpoint);

        // Add connection to controller with an expiration of -1, so the connection
        // won't expire.
        controller.add_connection(other_endpoint, -1);
    }
} }
