#ifndef PROF_TIME_H_
#define PROF_TIME_H_

#include <time.h>

#if defined __USE_POSIX199309
    typedef struct timespec prof_time;
#else
    #define prof_time clock_t
#endif


prof_time prof_zero_time();
prof_time prof_get_time();
prof_time prof_diff_time(prof_time start, prof_time end);
prof_time prof_add_time(prof_time t1, prof_time t2);
double prof_time_as_seconds(prof_time t);
double prof_time_as_milliseconds(prof_time t);
double prof_time_as_microseconds(prof_time t);
double prof_time_as_nanoseconds(prof_time t);

#endif /* PROF_TIME_H_ */