/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __LIB_CONCURRENT_RATE_LIMITER_HPP__
#define __LIB_CONCURRENT_RATE_LIMITER_HPP__

#include "Time.hpp"

namespace Concurrent
{
    /*!
     * This class makes it easy to perform an operation at a specific rate. After
     * starting some processing, the object will determine how much time to sleep
     * for to maintain a consistent processing rate.
     *
     * To use, reset the object before the first update. Then, at the end of each
     * update, just call the sleep routine. e.g.
     *
     *      RateLimiter limiter(10000);  // 10000us period, e.g. 10ms, or 100Hz.
     *
     *      limiter.reset();
     *      while (true)
     *      {
     *          process_stuff();
     *          limiter.sleep();
     *      }
     */
    class RateLimiter
    {
        public:
            /*!
             * Create the limiter with the specified period (not a frequency).
             * @param us_period The period in microseconds.
             */
            RateLimiter( const long us_period )
                : period_nsec(us_period*1000), deadline_nsec(0),
                    missed_deadlines(0)
            { }
            
            /*!
             * Reset the object. This just sets the last update time to the
             * current time.
             */
            inline void reset( void ) { deadline_nsec = Concurrent::ntime(); }

            /*!
             * Sleep until next update period. This will sleep the remaining
             * time until the next deadline. It will also keep track of missed
             * deadlines.
             */
            inline void sleep( void )
            {
                deadline_nsec += period_nsec;

                // Compare with current time.
                long time_nsec = Concurrent::ntime();
                if (time_nsec > deadline_nsec)
                {
                    ++missed_deadlines;
                    reset();
                }
                else Concurrent::nsleep_to(deadline_nsec);
            }

            /*!
             * Determine how many deadlines have been missed.
             * @return The number of missed deadlines.
             */
            inline int get_missed_deadlines( void ) const { return missed_deadlines; }

            /*!
             * Determine the period.
             * @return The period in micoseconds.
             */
            inline long get_period( void ) const { return period_nsec / 1000; }

        private:
            long period_nsec;
            long deadline_nsec;
            int missed_deadlines;
    };
}

#endif
