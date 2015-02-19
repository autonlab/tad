/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __Concurrent_Thread_hpp__
#define __Concurrent_Thread_hpp__

namespace al { namespace concurrent
{
    /*!
     * This class implements a generic thread. This makes it easy to implement
     * new threads as just the run_loop() function needs to be implemented in
     * derived classes.
     *
     * @note Your run_loop() implementation should periodically watch the
     *          stopped parameter to determine if the parent thread has
     *          requested your thread to stop processing.
     * @note Don't call run_loop() directly, otherwise it won't be in a new
     *          thread. Start a new thread with the start() function.
     */
    class Thread
    {
        public:
            Thread( void ) : thread(0), stopped(false), running(false) { }
            virtual ~Thread( void ) { }

            /*!
             * Start the thread. This will create the thread and tell it to
             * run the implemented run_loop() routine.
             * @return Zero if successful, otherwise an implementation-dependent
             *          (e.g. pthreads) error code.
             */
            int start( void );

            /*!
             * Stop the thread. This requests nicely that the thread cease
             * executing. Implementors of the run_loop() should ensure they
             * observe the stopped parameter occasionally to see that this
             * actually happens.
             *
             * This returns immediately. Wait for the thread to close with join().
             * @see join()
             */
            void stop( void );

            /*!
             * Wait for the thread's execution to end.
             */
            void * join( void );

            /*!
             * Returns the thread handle. It's meaning is implementation-dependent.
             * @return A pointer to the thread handle.
             */
            inline void * get_thread_handle( void ) const { return thread; }

            /*!
             * Determine whether or not the thread has left its run_loop() method.
             * @return True if the thread is still in the run_loop() method, false
             *          otherwise.
             */
            inline bool is_running( void ) const { return running; }

        protected:
            /*!
             * This is a helper function to launch the run_loop() routine in
             * derived classes. Don't try to call this directly!
             * @param context Expected to be a pointer to the Concurrent::Thread object.
             * @return The pointer to data returned from run_loop().
             * @note Don't use this function!
             */
            static void * run_loop_helper( void * context );

            /*!
             * This is the main loop routine to be implemented in derived classes.
             * This is the only function that needs to be implemented for a thread to
             * work. It's not called directly; run a new thread with the start() routine.
             * @return Any data you would like to return.
             */
            virtual void * run_loop( void ) = 0;

        protected:
            void * thread;
            bool stopped;

        private:
            bool running;
    };
} }

#endif
