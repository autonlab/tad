/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Connection_hpp__
#define __SRL_Connection_hpp__

#include <string>

namespace al { namespace srl
{
    /*!
     * This class provides an interface to generic connections.
     */
    class Connection
    {
        public:
            virtual ~Connection( void ) { disconnect(); }

            /*!
             * Send a message through the interface.
             *
             * @param message The message to send.
             * @return True if sent successfully, false otherwise.
             */
            virtual bool send( const std::string message ) = 0;

            /*!
             * Receive a message through the interface.
             *
             * @param [out] message The buffer in which to receive a message.
             * @return True if a message was received successfully, false otherwise.
             */
            virtual bool receive( std::string & message ) = 0;

            /*!
             * @return True if a message is available to be read, false otherwise.
             */
            virtual bool is_message_available( void ) = 0;

            /*!
             * @return True if the connection is connected to the other endpoint,
             *          false otherwise.
             */
            virtual bool is_connected( void ) = 0;

            /*!
             * Disconnect the interface from its endpoint.
             */
            virtual void disconnect( void ) { }
    };
} }

#endif
