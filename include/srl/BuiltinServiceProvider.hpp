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
            BuiltinServiceProvider( Controller & controller);

        private:
            void handle_NoOp( const InterfaceMessage & message, Connection * const client );
            void handle_Shutdown( const InterfaceMessage & message, Connection * const client );
            void handle_RegisterService( const InterfaceMessage & message, Connection * const client );

            virtual void handle_message( const InterfaceMessage & message, Connection * const client );

        private:
            Controller & controller;
    };
} }

#endif
