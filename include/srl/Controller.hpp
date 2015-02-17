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
    class Controller : public concurrent::Thread
    {
        public:
            void cleanup( void );

            bool register_interface( CommIF * const cif, const bool managed = true )
            {
                if (!cif) return false;
                comm_ifs.push_back(CommIFDescriptor(cif, managed));
            }

            void add_connection( Connection * connection ) { connections.push_front(connection); }
            concurrent::Queue<Connection *> & get_active_connections( void ) { return active_connections; }

            ServiceProvider * get_provider( const std::string name )
            {
                auto provider_iter = providers.find(name);
                return provider_iter != providers.end() ? provider_iter->second : 0;
            }

            void add_provider( ServiceProvider * const provider )
                { providers[provider->get_name()] = provider; }

        protected:
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
