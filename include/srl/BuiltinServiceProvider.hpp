/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Builtin_Service_Provider_hpp__
#define __SRL_Builtin_Service_Provider_hpp__

#include "srl/ServiceProvider.hpp"

namespace al { namespace srl
{
    class BuiltinServiceProvider : public ServiceProvider
    {
        public:
            BuiltinServiceProvider( Controller & controller) :
                controller(controller),
                ServiceProvider("Builtin", NULL)
            {
                add_service("Shutdown");
                add_service("RegisterService");
            }

        private:
            void handle_Shutdown( void );
            void handle_RegisterService( const InterfaceMessage & message, Connection * client );

            virtual void handle_message( const InterfaceMessage & message, Connection * client );

        private:
            Controller & controller;
    };
} }

#endif
