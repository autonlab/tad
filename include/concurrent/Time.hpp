/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __TIME_HPP__
#define __TIME_HPP__

namespace al { namespace concurrent
{
    /*!
     * Sleep to a specific clock time.
     * @param nano_time The clock time (in nanoseconds) to sleep to.
     */
    void nsleep_to( const long nano_time );

    /*!
     * The following routines call nsleep_to but allow you to enter a clock time in
     * microseconds (usleep_to()), milliseconds (msleep_to()), or seconds (ssleep_to()).
     * The input parameters are in those respective units.
     */

    inline void usleep_to( const long micro_time ) { nsleep_to(micro_time * 1000); }
    inline void msleep_to( const long milli_time ) { nsleep_to(milli_time * 1000000); }
    inline void ssleep_to( const long seconds_time ) { nsleep_to(seconds_time * 1000000000); }

    /*!
     * Sleep for the specified duration of time.
     * @param nano_seconds The duration (in nanoseconds) to sleep.
     */
    void nsleep( const long nano_seconds );

    /*!
     * The following routines call nsleep but allow you to enter a duration in
     * microseconds (usleep()), milliseconds (msleep()), or seconds (ssleep()).
     * The input parameters are in the respective units.
     */

    inline void usleep( const long micro_seconds ) { nsleep(micro_seconds * 1000); }
    inline void msleep( const long milli_seconds ) { nsleep(milli_seconds * 1000000); }
    inline void ssleep( const long seconds ) { nsleep(seconds * 1000000000); }

    /*!
     * Determine the current clock time.
     * @return The current clock time (in nanoseconds).
     * @note For consistency in an application, this does not use the adjustable
     *          "wall clock" time. Instead, it uses a monolithic clock that (should be)
     *          guarenteed always to increase and represent the total running time
     *          of the application. Don't use this if you need to compare real times
     *          or even times between forked processes because the initial offset will
     *          almost definitely differ!
     */
    long ntime( void );

    /*!
     * The following routines call ntime but allow you to receive a clock time in
     * microseconds (utime()), milliseconds (mtime()), or seconds (stime()).
     * The return values are those respective units.
     */

    inline long utime( void ) { return ntime()/1000; }
    inline long mtime( void ) { return ntime()/1000/1000; }
    inline long stime( void ) { return ntime()/1000/1000/1000; }
} }

#endif
