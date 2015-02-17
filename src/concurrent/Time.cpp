/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#include "concurrent/Time.hpp"

#include <time.h>

namespace al { namespace concurrent
{
    void nsleep_to( const long nano_time )
    {
        timespec wakeup_time;
        wakeup_time.tv_sec  = nano_time / 1000000000;
        wakeup_time.tv_nsec = nano_time % 1000000000;
        if (!clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL)); // ATW: Fail silently...
    }

    void nsleep( const long nano_seconds )
    {
        timespec sleep;
        sleep.tv_sec  = nano_seconds / 1000000000;
        sleep.tv_nsec = nano_seconds % 1000000000;
        if (!nanosleep(&sleep, &sleep)); // ATW: Fail silently..
    }

    long ntime( void )
    {
        timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        return time.tv_sec*1000000000 + time.tv_nsec;
    }
} }
