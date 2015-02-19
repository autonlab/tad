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
        CallbackConnection * other_endpoint = new CallbackConnection;
        endpoint.connect(other_endpoint);
        other_endpoint->connect(&endpoint);
        controller.add_connection(other_endpoint);
    }
} }
