/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Loopback_Connection_hpp__
#define __SRL_Loopback_Connection_hpp__

#include "srl/Connection.hpp"
#include "concurrent/Queue.hpp"

namespace al { namespace srl
{
    /*!
     * This class implements a loopback connection, who is considered both endpoints.
     *
     * For undocumented methods, @see Connection.
     */
    class LoopbackConnection : public Connection
    {
        public:
            LoopbackConnection( void ) { }

            virtual bool send( const std::string message ) { handle(message); }
            virtual bool receive( std::string & message ) { return in.pop(message); }
            virtual bool is_message_available( void ) { return in.size() > 0; }
            virtual bool is_connected( void ) { return true; }
            virtual void disconnect( void ) { }

        protected:
            virtual void handle( const std::string message ) { in.push(message); }

        private:
            concurrent::Queue<std::string> in;
    };
} }

#endif
