/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Callback_Interface_hpp__
#define __SRL_Callback_Interface_hpp__

#include "srl/CommunicationInterface.hpp"
#include "srl/CallbackConnection.hpp"

namespace al { namespace srl
{
    /*!
     * This class implements a simple interface to handle class callbacks.
     */
    class CallbackInterface : public CommIF
    {
        public:
            CallbackInterface( Controller & controller );

            /*!
             * Connects to an endpoint. A new CallbackConnection is created and
             * connected to the connection specified on the input.
             *
             * @param The endpoint to connect to.
             */
            void connect( CallbackConnection & endpoint );
    };
} }

#endif
