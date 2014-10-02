#include <time.h>
#include "prof_time.h"

#if defined __USE_POSIX199309

    prof_time prof_zero_time() {
        prof_time result;
        result.tv_nsec = 0;
        result.tv_sec = 0;
        return result;
    }

    prof_time prof_get_time() {
        prof_time result;
        clock_gettime(CLOCK_REALTIME, &result);
        return result;
    }

    prof_time prof_diff_time(prof_time start, prof_time end) {
        prof_time result;

        if (end.tv_nsec < start.tv_nsec) {
            result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
            result.tv_sec = end.tv_sec - 1 - start.tv_sec;
        } else {
            result.tv_nsec = end.tv_nsec - start.tv_nsec;
            result.tv_sec = end.tv_sec - start.tv_sec;
        }

        return result;
    }

    prof_time prof_add_time(prof_time t1, prof_time t2) {
        prof_time result;

        result.tv_nsec = t1.tv_nsec + t2.tv_nsec;
        result.tv_sec = t1.tv_sec + t2.tv_sec;

        if (result.tv_nsec > 1000000000) {
            result.tv_nsec -=1000000000;
            result.tv_sec++;
        }

        return result;
    }

    double prof_time_as_seconds(prof_time t) {
        return ((double)t.tv_nsec)/1000000000 + (double)t.tv_sec;
    }

    double prof_time_as_milliseconds(prof_time t) {
        return ((double)t.tv_nsec)/1000000 + (double)t.tv_sec*1000;
    }

    double prof_time_as_microseconds(prof_time t) {
        return ((double)t.tv_nsec)/1000 + (double)t.tv_sec*1000000;
    }

    double prof_time_as_nanoseconds(prof_time t) {
        return ((double)t.tv_nsec) + (double)t.tv_sec*1000000000;
    }

#else

    prof_time prof_zero_time() {
        return 0;
    }

    prof_time prof_get_time() {
        return clock();
    }

    prof_time prof_diff_time(prof_time start, prof_time end) {
        return end - start;
    }

    prof_time prof_add_time(prof_time t1, prof_time t2) {
        return t1 + t2;
    }

    double prof_time_as_seconds(prof_time t) {
        return ((double)t) / CLOCKS_PER_SEC;
    }

    double prof_time_as_milliseconds(prof_time t) {
        return ((double)t*1000) / CLOCKS_PER_SEC;
    }

    double prof_time_as_microseconds(prof_time t) {
        return ((double)t*1000000) / CLOCKS_PER_SEC;
    }

    double prof_time_as_nanoseconds(prof_time t) {
        return ((double)t*1000000000) / CLOCKS_PER_SEC;
    }
#endif