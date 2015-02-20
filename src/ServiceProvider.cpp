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
    void ServiceProvider::handle_message( const InterfaceMessage & message, Connection * const client )
        { connection->send(message.encode()); }
} }
