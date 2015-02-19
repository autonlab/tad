/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Router_hpp__
#define __SRL_Router_hpp__

#include <concurrent/Thread.hpp>

namespace al { namespace srl
{
    class Controller;

    /*!
     * This class implements a router thread. This will handle active connections by
     * receiving messages, checking validity, and passing them along to the relevant
     * services. This will also handle the easy error cases, like unparsable messages
     * or unregistered services or modules.
     */
    class Router : public concurrent::Thread
    {
        public:
            /*!
             * @param controller The parent controller object.
             */
            Router( Controller & controller ) : controller(controller) { }

        protected:
            virtual void * run_loop( void );

        private:
            Controller & controller;
    };
} }

#endif
