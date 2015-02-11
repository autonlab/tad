/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __LIB_CONCURRENT_RATE_MONITOR_HPP__
#define __LIB_CONCURRENT_RATE_MONITOR_HPP__

#include "Time.hpp"

namespace Concurrent
{
    /*!
     * This class is useful for timing an event. The use is simple: reset before
     * the event you want to time, then mark after each event. The marks will
     * give you the incremental changes from one mark to the next. You can also
     * get the total elapsed time since the start.
     */
    class RateMonitor
    {
        public:
            RateMonitor( void )
                : last_time_nsec(0), elapsed_nsec(0)
            { }

            /*!
             * Reset the event start time to the current time.
             */
            inline void reset( void )
            {
                last_time_nsec = Concurrent::ntime();
                elapsed_nsec = 0;
            }

            /*!
             * Mark the current time. This will store the time of the last event.
             * @return The time in nanoseconds since the last event (call to mark(),
             *          or reset() if mark() has not yet been called).
             */
            inline long mark( void )
            {
                // Compare with current time.
                long time_nsec = Concurrent::ntime();
                long nsec_diff = time_nsec - last_time_nsec;
                last_time_nsec = time_nsec;
                elapsed_nsec += nsec_diff;
                return nsec_diff;
            }

            /*!
             * Return the time elapsed since the last mark and the reset.
             * @return The time elapsed in nanoseconds.
             */
            inline long get_elapsed_time( void ) const { return elapsed_nsec; }
        
        private:
            long last_time_nsec;
            long elapsed_nsec;
    };
}

#endif
