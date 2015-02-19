/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "srl/ServiceProvider.hpp"

#include "concurrent/Time.hpp"
#include "srl/BuiltinMessageFactory.hpp"

namespace al { namespace srl
{
    void ServiceProvider::handle_message(
            const InterfaceMessage & message,
            Connection * client )
    {
        // Forward on connection.
        connection->send(message.encode());

        // Wait for a response.
        std::string response = "";
        int tries = 500;
        while (!connection->receive(response) && (tries-- > 0)) concurrent::msleep(10);
        if (response != "") client->send(response);
        else client->send(ErrorMessageFactory::generate(
                    message, "Service provider gave no response; timeout."));
    }
} }
