#ifndef __SRL_Service_Provider_hpp__
#define __SRL_Service_Provider_hpp__

#include "concurrent/Time.hpp"

#include "srl/Connection.hpp"
#include "srl/Message.hpp"
#include "srl/BuiltinMessageFactory.hpp"

#include <set>
#include <string>

namespace al { namespace srl
{
    class Controller;

    class ServiceProvider
    {
        public:
            ServiceProvider( const std::string name, Connection * connection ) :
                name(name), connection(connection)
            { }

            virtual void handle_message( const InterfaceMessage & message, Connection * client )
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

            std::string get_name( void ) const { return name; }
            Connection * get_connection( void ) const { return connection; }

            void add_service( const std::string service ) { services.insert(service); }
            bool is_service_available( const std::string service )
                { return services.find(service) != services.end(); }

        private:
            std::string name;
            std::set<std::string> services;
            Connection * connection;
    };

    class BuiltinServiceProvider : public ServiceProvider
    {
        public:
            BuiltinServiceProvider( Controller & controller) :
                controller(controller),
                ServiceProvider("Builtin", NULL)
            {
                add_service("RegisterService");
            }

        private:
            void handle_RegisterService( const InterfaceMessage & message, Connection * client );

            virtual void handle_message( const InterfaceMessage & message, Connection * client );

        private:
            Controller & controller;
    };
} }

#endif
