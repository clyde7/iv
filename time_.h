#ifndef TIME_H
#define TIME_H

#ifdef WINDOWS
#include <winsock.h>
#else
#include <sys/time.h>
#endif

typedef struct timeval Time_;

extern Time_ const ZERO_TIME;
extern Time_ START_TIME;

Time_  time_current(void);
float time_difference(Time_, Time_);
float time_duration(Time_);
void  time_sleep(int micro_seconds);
float time_update_fps(void);

Time_  time_from_duration(float);
void  time_increment(Time_);
void  time_force(Time_);
void  time_unforce(void);

#endif
