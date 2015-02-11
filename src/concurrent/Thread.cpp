/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "concurrent/Thread.hpp"
#include <pthread.h>

namespace Concurrent
{
    int Thread::start( void )
    {
        thread = new pthread_t;

        int result = -1;
        pthread_attr_t attributes;
        if (!pthread_attr_init(&attributes))
        {
            pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE);

            result = pthread_create(
                    reinterpret_cast<pthread_t *>(thread),
                    &attributes,
                    &Concurrent::Thread::run_loop_helper,
                    reinterpret_cast<void *>(this));
            pthread_attr_destroy(&attributes);
        }

        if (result)
        {
            delete reinterpret_cast<pthread_t *>(thread);
            thread = 0;
            stopped = false;
            running = false;
        }

        return result;
    }

    void Thread::stop( void )
    {
        stopped = true;
    }

    void * Thread::join( void )
    {
        void * result = 0;
        if (thread) pthread_join(*reinterpret_cast<pthread_t *>(thread), &result);
        return result;
    }

    void * Thread::run_loop_helper( void * context )
    {
        Thread * thread = reinterpret_cast<Thread *>(context);
        thread->running = true;
        void * result = thread->run_loop();
        thread->stopped = true;

        delete reinterpret_cast<pthread_t *>(thread->thread);
        thread->thread = 0;

        thread->running = false;
        pthread_exit(result);
        return result; // Shouldn't get here.
    }
}
