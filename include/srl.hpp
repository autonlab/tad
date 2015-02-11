#ifndef __SRL__hpp__
#define __SRL__hpp__

#include "concurrent/Thread.hpp"
#include "concurrent/Queue.hpp"

#include <map>

namespace al { namespace srl
{
    class Connection
    {
    };

    class CommIF
    {
    };

    class CallbackInterface : public CommIF
    {
    };

    class CommIFDescriptor
    {
        public:
            CommIFDescriptor( CommIF * const cif, const bool managed ) :
                interface(cif), managed(managed)
            { }

            inline CommIF * get_interface( void ) const { return interface; }
            inline bool is_managed( void ) const { return managed; }

        private:
            CommIF * const interface;
            const bool managed;
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

        protected:
            virtual void * run_loop( void )
            {
            }

        private:
            concurrent::Queue<Connection *>             client_connections;
            concurrent::Queue<Connection *>             service_connections;
            std::vector<CommIFDescriptor>               comm_ifs;
            std::vector<concurrent::Thread *>           routers;
            std::map<std::string, ServiceProvider *>    providers;
    };
} }

#endif
