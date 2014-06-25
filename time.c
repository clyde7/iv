#include <stdlib.h>
#ifndef WINDOWS
#include <unistd.h>
#endif

#include "math_.h"
#include "time_.h"

#ifdef WINDOWS
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock.h>

static void gettimeofday(struct timeval * t, void * timezone)
{
    struct _timeb timebuffer;
    _ftime(&timebuffer);

    t->tv_sec=timebuffer.time;
    t->tv_usec=1000*timebuffer.millitm;
}
#endif

Time_ const ZERO_TIME = {0, 0};
Time_ START_TIME = {0, 0};

static int forced;
static Time_ forced_time;

Time_ time_current(void)
{
    if (forced)
        return forced_time;

    Time_ time;
    gettimeofday(&time, NULL);

    return time;
}

float time_difference(Time_ begin, Time_ end)
{
    return
        (end.tv_sec  - begin.tv_sec) +
        (end.tv_usec - begin.tv_usec) * 1E-6;
}

float time_duration(Time_ begin)
{
    return time_difference(begin, time_current());
}

void time_sleep(int micro_seconds)
{
#ifdef WINDOWS
	Sleep(micro_seconds / 1000);
#else
    usleep(micro_seconds);
#endif
}

float time_update_fps(void)
{
    static float fps = 0;
    static Time_ time;
    static int frame_count = 0;

    float duration = time_duration(time);
    if (duration >= 0.1)
    {
        fps = round(frame_count / duration);
        time = time_current();
        frame_count = 0;
    }

    ++ frame_count;

    return fps;
}

Time_ time_from_duration(float duration)
{
    float seconds = floor(duration);
    Time_ time =
    {
        seconds,
        floor((duration - seconds) * 1000000)
    };

    return time;
}

void time_increment(Time_ time)
{
    forced_time.tv_sec  += time.tv_sec;
    forced_time.tv_usec += time.tv_usec;

    if (forced_time.tv_usec > 1000000)
        forced_time.tv_usec -= 1000000;
}

void time_force(Time_ time)
{
    forced = 1;
    forced_time = time;
}

void time_unforce(void)
{
    forced = 0;
}
