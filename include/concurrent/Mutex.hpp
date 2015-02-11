/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __LIB_CONCURRENT_MUTEX_HPP__
#define __LIB_CONCURRENT_MUTEX_HPP__

namespace al { namespace concurrent
{
    /*!
     * This is a generic mutual exclusion (mutex) construct.
     */
    class Mutex
    {
        public:
            /*
             * Create the mutex.
             * @param shared_address The address where the mutex should be initialized.
             *          This is for initializing a mutex in shared memory if the
             *          mutex needs to be shared between processes. If NULL the
             *          mutex can only be shared among threads within the creating
             *          process.
             * @note If the placement new is used to initialize the mutex, remember
             *          to call the destructor explicitely.
             * @see ~Mutex()
             */
            Mutex( void * const shared_address = 0 );

            /*
             * Destructor.
             * @note In cases where a placement new is used to put the mutex in
             *          shared memory, the destructor must be called explicitely.
             */
            ~Mutex( void );

            /*!
             * Returns the size (in bytes) required to store a mutex object.
             * Useful for allocating space in shared memory to store the mutex.
             * @return Size (in bytes) needed to store mutex.
             */
            static int get_required_size( void );

            /*!
             * Determine whether or not the mutex was allocated successfully.
             * @return True if the mutex is valid, false otherwise.
             */
            inline bool is_valid( void ) const { return mutex != 0; }

            /*!
             * Determine whether or not the mutex needs to be freed explicitely.
             * This will return true if the shared address is not NULL, since it
             * is assumed you used a placement new to construct the object.
             * @return True if the object needs an explicit destructor call, false
             *          otherwise.
             */
            inline bool needs_explicit_free( void ) const
            {
                return shared_address != 0;
            }

            /*!
             * Lock the mutex.
             * @return Zero on success or an implementation-dependent (e.g. pthread)
             *          error code.
             */
            int lock( void );

            /*!
             * Unlock the mutex.
             * @return Zero on success or an implementation-dependent (e.g. pthread)
             *          error code.
             */
            int unlock( void );

        public:
            void * const shared_address;
            void * mutex;
    };

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
