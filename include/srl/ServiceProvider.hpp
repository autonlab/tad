/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Service_Provider_hpp__
#define __SRL_Service_Provider_hpp__

#include "srl/Connection.hpp"
#include "srl/Message.hpp"

#include <set>
#include <string>

namespace al { namespace srl
{
    class Controller;
    class Log;

    class ServiceProvider
    {
        public:
            ServiceProvider( const std::string name, Connection * const connection ) :
                name(name), connection(connection)
            { }

            virtual void handle_message(
                    const InterfaceMessage & message,
                    Connection * const client,
                    Log & log );

            std::string get_name( void ) const { return name; }
            Connection * get_connection( void ) const { return connection; }

            void add_service( const std::string service ) { services.insert(service); }
            bool is_service_available( const std::string service )
                { return services.find(service) != services.end(); }

        protected:
            std::string name;
            std::set<std::string> services;
            Connection * connection;
    };
} }

#endif
