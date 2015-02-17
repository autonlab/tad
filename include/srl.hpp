#ifndef __SRL__hpp__
#define __SRL__hpp__

#include "concurrent/Time.hpp"
#include "concurrent/Thread.hpp"
#include "concurrent/Queue.hpp"
#include "srl/Message.hpp"

#include <map>
#include <list>
#include <string>

#include <iostream>

namespace al { namespace srl
{
    class Connection
    {
        public:
            // TODO: Maybe make this two states?
            typedef enum
            {
                StateUnknown            = 0,
                
                StateIdle               = 1,
                StateMessageAvailable   = 2,
                StateWaitingForResponse = 3,

                StateCount
            } State;

        public:
            Connection( void ) : state(StateIdle) { }

            virtual bool send( const std::string message ) = 0;
            virtual bool receive( std::string & message ) = 0;
            virtual bool is_connected( void ) = 0;
            virtual void disconnect( void ) = 0;

            State get_state( void ) const { return state; }

        protected:
            State state;
    };

    class CallbackConnection : public Connection
    {
        public:
            CallbackConnection( void ) : other(0) { }
            ~CallbackConnection( void ) { disconnect(); }

            virtual bool send( const std::string message )
            {
                if (other)
                {
                    state = Connection::StateWaitingForResponse;
                    other->handle(message);
                }
                else return other != 0;
            }

            virtual bool receive( std::string & message )
            {
                bool message_popped = in.pop(message);
                state = Connection::StateIdle;
                return message_popped;
            }

            virtual bool is_connected( void ) { return other != 0; }

            void connect( CallbackConnection * other ) { this->other = other; }
            virtual void disconnect( void )
            {
                if (other) other->other = 0;
                other = 0;
            }

        protected:
            virtual void handle( const std::string message )
            {
                state = Connection::StateMessageAvailable;
                in.push(message);
            }

        private:
            CallbackConnection * other;
            concurrent::Queue<std::string> in;
    };

    class CommIF
    {
        public:

        protected:
            
    };

    class CallbackInterface : public CommIF
    {
        public:
            Connection * connect( CallbackConnection & endpoint )
            {
                CallbackConnection * other_endpoint = new CallbackConnection;
                endpoint.connect(other_endpoint);
                other_endpoint->connect(&endpoint);
                return other_endpoint;
            }
    };

    class CommIFDescriptor
    {
        public:
            CommIFDescriptor( void ) : interface(0), managed(false) { }

            CommIFDescriptor( CommIF * const cif, const bool managed ) :
                interface(cif), managed(managed)
            { }

            inline CommIF * get_interface( void ) const { return interface; }
            inline bool is_managed( void ) const { return managed; }

        private:
            CommIF * interface;
            bool managed;
    };

    class ServiceProvider
    {
        private:
            std::map<std::string, bool> services;
    };

    class Controller : public concurrent::Thread
    {
        public:
            void cleanup( void )
            {
                // Free any managed
            }

            bool register_interface( CommIF * const cif, const bool managed = true )
            {
                if (!cif) return false;
                comm_ifs.push_back(CommIFDescriptor(cif, managed));
            }

            void add_connection( Connection * connection )
            {
                connections.push_front(connection);
            }

        protected:
            virtual void * run_loop( void )
            {
                while (true)
                {
                    // Make sure we're not stopped.
                    if (stopped) break;

                    // Loop through connections looking for messages and removing disconnected
                    // connections.
                    auto iter = connections.begin();
                    while (iter != connections.end())
                    {
                        Connection * connection = *iter;

                        // If connection was disconnected, destroy it.
                        if (!connection->is_connected())
                        {
                            delete connection;
                            connections.erase(iter++);
                        }
                        else
                        {
                            // Data is available?
                            if (connection->get_state() == Connection::StateMessageAvailable)
                                active_connections.push(connection);
                            ++iter;
                        }
                    }

                    // TODO Move this part to workers.
                    // Handle any active conections.
                    Connection * connection;
                    while (active_connections.pop(connection))
                    {
                        // Get the message.
                        std::string raw_message;
                        if (connection->receive(raw_message))
                        {
                            // Parse message.
                            InterfaceMessage message;
                            message.decode(raw_message);
                            if (message.is_valid())
                                std::cout << "Parsed message okay:\n" << message.encode() << std::endl;
                            else std::cout << "Pessage invalid:\n" << message.get_last_error() << std::endl;
                            concurrent::ssleep(2);
                            connection->send("This is no real JSON!");
                            connection->disconnect();
                        }

                        // Could not get a valid message.
                        else std::cout << "Could not get valid message." << std::endl;
                    }
                }
            }

        private:
            std::list<Connection *>                     connections;
            concurrent::Queue<Connection *>             active_connections;

            std::vector<CommIFDescriptor>               comm_ifs;
            std::vector<concurrent::Thread *>           routers;
            std::map<std::string, ServiceProvider *>    providers;
    };
} }

#endif
