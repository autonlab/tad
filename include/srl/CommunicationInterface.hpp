#ifndef __SRL_Communication_Interface_hpp__
#define __SRL_Communication_Interface_hpp__

#include "srl/Connection.hpp"

namespace al { namespace srl
{

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
} }

#endif
