/*
   Date:         February 18, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __SRL_Communication_Interface_hpp__
#define __SRL_Communication_Interface_hpp__

#include "srl/Connection.hpp"

namespace al { namespace srl
{
    class Controller;

    /*!
     * This class provides an interface for communication interfaces.
     */
    class CommIF
    {
        public:
            CommIF( Controller & controller ) : controller(controller) { }
            virtual ~CommIF( void ) { }

        protected:
            Controller & controller;
    };

    /*!
     * This class represents a descriptor, allowing the system controller to store some
     * additional metadata with the interface handles.
     */
    class CommIFDescriptor
    {
        public:
            CommIFDescriptor( void ) : interface(0), managed(false) { }

            /*!
             * @param cif The communication interface.
             * @param managed If true, the controller will handle destroying the object
             *          when it closes.
             */
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
