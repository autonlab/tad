#ifndef __SRL_Router_hpp__
#define __SRL_Router_hpp__

#include <concurrent/Thread.hpp>

namespace al { namespace srl
{
    class Controller;
    class Router : public concurrent::Thread
    {
        public:
            Router( Controller & controller ) : controller(controller) { }

        protected:
            virtual void * run_loop( void );

        private:
            Controller & controller;
    };
} }

#endif
