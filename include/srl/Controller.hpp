/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Controller_hpp__
#define __SRL_Controller_hpp__

#include "concurrent/Thread.hpp"
#include "concurrent/Queue.hpp"

#include "srl/Connection.hpp"
#include "srl/CommunicationInterface.hpp"
#include "srl/ServiceProvider.hpp"
#include "srl/Router.hpp"

#include <list>
#include <map>

namespace al { namespace srl
{
    /*!
     * This class implements a thread that handles connections and services. It
     * also controls the message routers.
     */
    class Controller : public concurrent::Thread
    {
        public:
            /*!
             * Register an interface with the controller.
             *
             * @param cif The interface to register.
             * @param managed If true, the controller will destroy the interface at shutdown.
             * @return True if successfully registered, false otherwise.
             */
            bool register_interface( CommIF * const cif, const bool managed = true )
            {
                if (!cif) return false;
                comm_ifs.push_back(CommIFDescriptor(cif, managed));
            }

            /*!
             * Add a connection to the pool.
             *
             * @param connection The connection to add.
             */
            void add_connection( Connection * connection ) { connections.push_front(connection); }

            /*!
             * Get the queue of active connections. These are the connections which have some
             * data available to be read and processed.
             *
             * @return The thread-safe queue containing the connections.
             */
            concurrent::Queue<Connection *> & get_active_connections( void ) { return active_connections; }

            /*!
             * Get the provider interface by specifying the module name.
             *
             * @param name The name of the module.
             * @return A pointer to the provider interface if registered, NULL otherwise.
             */
            ServiceProvider * get_provider( const std::string name )
            {
                auto provider_iter = providers.find(name);
                return provider_iter != providers.end() ? provider_iter->second : 0;
            }

            /*!
             * Register a provider with the controller.
             *
             * @param provider The provider interface to register.
             */
            void register_provider( ServiceProvider * const provider )
                { providers[provider->get_name()] = provider; }

        protected:
            void cleanup( void );

            virtual void * run_loop( void );

        private:
            std::list<Connection *>                     connections;
            concurrent::Queue<Connection *>             active_connections;

            std::vector<CommIFDescriptor>               comm_ifs;
            std::vector<concurrent::Thread *>           routers;
            std::map<std::string, ServiceProvider *>    providers;
    };
} }

#endif
