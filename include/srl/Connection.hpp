#ifndef __SRL_Connection_hpp__
#define __SRL_Connection_hpp__

#include "concurrent/Queue.hpp"

#include <string>

namespace al { namespace srl
{
    class Connection
    {
        public:
            virtual bool send( const std::string message ) = 0;
            virtual bool receive( std::string & message ) = 0;
            virtual bool is_message_available( void ) = 0;
            virtual bool is_connected( void ) = 0;
            virtual void disconnect( void ) = 0;
    };

    class CallbackConnection : public Connection
    {
        public:
            CallbackConnection( void ) : other(0) { }
            ~CallbackConnection( void ) { disconnect(); }

            virtual bool send( const std::string message )
            {
                if (other) other->handle(message);
                else return other != 0;
            }

            virtual bool receive( std::string & message )
            {
                bool message_popped = in.pop(message);
                return message_popped;
            }

            virtual bool is_message_available( void ) { return in.size() > 0; }

            virtual bool is_connected( void ) { return other != 0; }

            void connect( CallbackConnection * other ) { this->other = other; }
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
