/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Callback_Connection_hpp__
#define __SRL_Callback_Connection_hpp__

#include "srl/Connection.hpp"
#include "concurrent/Queue.hpp"

namespace al { namespace srl
{
    /*!
     * This class implements a connection that can be used without a real physical communication
     * medium like a network connection, instead overriding the message handling function to
     * perform services immediately. This requires two CallbackConnections be made in the code
     * so there are effectively two endpoints, just like there would be with other connection
     * types. CallbackInterface handles creating this other endpoint, which is managed by the
     * controller.
     *
     * For undocumented methods, @see Connection.
     */
    class CallbackConnection : public Connection
    {
        public:
            CallbackConnection( void ) : other(0) { }

            virtual bool send( const std::string message )
            {
                if (other) other->handle(message);
                else return other != 0;
            }

            virtual bool receive( std::string & message ) { return in.pop(message); }

            virtual bool is_message_available( void ) { return in.size() > 0; }

            virtual bool is_connected( void ) { return other != 0; }

            /*!
             * Connect to another CallbackConnection endpoint.
             *
             * @param other The other endpoint.
             */
            void connect( CallbackConnection * const other ) { this->other = other; }

            virtual void disconnect( void )
            {
                if (other) other->other = 0;
                other = 0;
            }

        protected:
            virtual void handle( const std::string message ) { in.push(message); }

        private:
            CallbackConnection * other;
            concurrent::Queue<std::string> in;
    };
} }

#endif
