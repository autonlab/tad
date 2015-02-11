/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "concurrent/Mutex.hpp"

#include <pthread.h>

namespace Concurrent
{
    Mutex::Mutex( void * const shared_address )
        : shared_address(shared_address), mutex(0)
    {
        pthread_mutexattr_t attributes;
        if (pthread_mutexattr_init(&attributes))
            throw "Unable to initialize mutex attributes";

        if (shared_address)
        {
            mutex = shared_address;

            if (pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED))
                throw "Unable to set mutex attribute PTHREAD_PROCESS_SHARED";
        }
        else mutex = new pthread_mutex_t;
        if (pthread_mutex_init(reinterpret_cast<pthread_mutex_t *>(mutex), &attributes))
        {
            if (!shared_address)
                delete reinterpret_cast<pthread_mutex_t *>(mutex);
            mutex = 0;
            throw "Unable to initialize mutex.";
        }
    }

    Mutex::~Mutex( void )
    {
        if (mutex)
        {
            pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(mutex));
            if (!shared_address)
                delete reinterpret_cast<pthread_mutex_t *>(mutex);
        }
    }

    int Mutex::get_required_size( void )
    {
        return sizeof(pthread_mutex_t);
    }

    int Mutex::lock( void )
    {
        return pthread_mutex_lock(reinterpret_cast<pthread_mutex_t *>(mutex));
    }

    int Mutex::unlock( void )
    {
        return pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t *>(mutex));
    }
}
