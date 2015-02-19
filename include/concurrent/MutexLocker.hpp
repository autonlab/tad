/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __Concurrent_Mutex_Locker_hpp__
#define __Concurrent_Mutex_Locker_hpp__

#include "concurrent/Mutex.hpp"

namespace al { namespace concurrent
{
    /*!
     * This is a convinience class that can be used to automatically lock a mutex
     * on creation and unlock when the object goes out of scope (e.g. at the end of
     * a function call). This makes writing mutex locking code in functions with
     * many returns much less cumbersom, e.g.
     *
     *      int my_critical_section( void )
     *      {
     *          MutexLocker locker(my_mutex);
     *          ...
     *          if (...) return 12;             // Notice no unlock call required.
     *          ...
     *          else { return -1; }
     *          ...
     *          return 0;
     *      }
     */
    class MutexLocker
    {
        public:
            /*!
             * Construct the locker with a reference to the mutex you wish to lock.
             * @param mutex A reference to the mutex to lock.
             */
            MutexLocker( Mutex & mutex )
                : mutex(mutex)
            {
                mutex.lock();
            }

            /*!
             * Destroy the locker, first unlocking the mutex.
             */
            ~MutexLocker( void )
            {
                mutex.unlock();
            }

        protected:
            Mutex & mutex;
    };
} }

#endif
