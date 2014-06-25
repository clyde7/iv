#include <stdio.h>
#include <time.h>

#include "error.h"
#include "math_.h"
#include "print.h"

typedef struct {float value; char * unit;} Scale;

void print_indentation(int level)
{
    static char const indentation[] = "  ";

    for (int i = 0; i != level; ++ i)
    {
        printf(indentation);
    }
}

static Scale unit_units(float value)
{
    float base = floor((float) (log(value) / log(10.0f)));

    if (base >= 9) { Scale s = {value / 1E9, "billion"}; return s; }
    if (base >= 6) { Scale s = {value / 1E6, "million"}; return s; }
    if (base >= 3) { Scale s = {value / 1E3, "thousand"}; return s; }

    Scale s = {value, ""}; return s;
}

static Scale unit_memory(float value)
{
    float t;
   
    t = value / (1024 * 1024 * 1024); if (t >= 1) { Scale s = {t, "GB"};    return s; }
    t = value / (1024 * 1024);        if (t >= 1) { Scale s = {t, "MB"};    return s; }
    t = value / 1024;                 if (t >= 1) { Scale s = {t, "KB"};    return s; }
    t = value;                                      Scale s = {t, "bytes"}; return s;
}

static Scale unit_frequency(float value)
{
    float base = floor(log(value) / log(10.0f));

    if (base >= 9) { Scale s = {value / 1E9, "GHz"}; return s; }
    if (base >= 6) { Scale s = {value / 1E6, "MHz"}; return s; }
    if (base >= 3) { Scale s = {value / 1E3, "kHz"}; return s; }

    Scale s = {value, "Hz"}; return s;
}

static Scale unit_time(float value)
{
    float t;
   
    t = value / 3600; if (fabs(t) >= 1) { Scale s = {t, "hours"};   return s; }
    t = value /   60; if (fabs(t) >= 1) { Scale s = {t, "minutes"}; return s; }
    t = value;        if (fabs(t) >= 1) { Scale s = {t, "seconds"}; return s; }
    t = value * 1E3;  if (fabs(t) >= 1) { Scale s = {t, "ms"};      return s; }
    t = value * 1E6;  if (fabs(t) >= 1) { Scale s = {t, "us"};      return s; }
    t = value * 1E9;                      Scale s = {t, "ns"};      return s;
}

void fprint_units(FILE * file, float value)
{
    Scale scaled = unit_units(value);
    fprintf(file, "%.2f %s", scaled.value, scaled.unit);
}

void print_units(float value)
{
    Scale scaled = unit_units(value);
    printf("%.2f %s", scaled.value, scaled.unit);
}

int sprintf_units(char * buffer, char const * format, float value)
{
    Scale scaled = unit_units(value);
    return sprintf(buffer, format, scaled.value, scaled.unit);
}

int sprint_units(char * buffer, float value)
{
    return sprintf_units(buffer, "%.2f %s", value);
}

void print_frequency(float value)
{
    Scale scaled = unit_frequency(value);
    printf("%.1f %s", scaled.value, scaled.unit);
}

int sprint_frequency(char * buffer, float value)
{
    Scale scaled = unit_frequency(value);
    return sprintf(buffer, "%.1f %s", scaled.value, scaled.unit);
}

void print_memory(float value)
{
    Scale scaled = unit_memory(value);
    printf("%.1f %s", scaled.value, scaled.unit);
}

int sprint_memory(char * buffer, float value)
{
    Scale scaled = unit_memory(value);
    return sprintf(buffer, "%.1f %s", scaled.value, scaled.unit);
}

void print_time(float value)
{
    Scale scaled = unit_time(value);
    printf("%.2f %s", scaled.value, scaled.unit);
}

int sprintf_time(char * buffer, char const * format, float value)
{
    Scale scaled = unit_time(value);
    return sprintf(buffer, format, scaled.value, scaled.unit);
}

int sprint_time(char * buffer, float value)
{
    return sprintf_time(buffer, "%.2f %s", value);
}

int sprint_time_classic(char * buffer, float value)
{
    int seconds = (int) value;
    int minutes = seconds / 60;
    int hours = minutes / 60;

    seconds = (int) value % 60;

    return sprintf(buffer, "%d:%02d:%02d", hours, minutes, seconds);
}

void print_array(char c, void * data, int count)
{
    for (int i = 0; i != count; ++ i)
    {
        if (i != 0)
            printf(", ");

        switch (c)
        {
            case 'd': printf("%d",   ((int *)      data)[i]); break;
            case 'f': printf("%f",   ((float *)    data)[i]); break;
            case 'x': printf("0x%x", ((unsigned *) data)[i]); break;
            default:  error_check(1, "unsupported print argument");
        }
    }

    puts("");
}

void fprint_date(FILE * file)
{
    time_t time_ = time(NULL);
    struct tm * t = localtime(&time_);

    fprintf(file, "%d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

void print_log_time(float value)
{
    int hours       = floor(value / 3600); value -= hours * 3600;
    int minutes     = floor(value / 60);   value -= minutes * 60;
    int seconds     = floor(value);        value -= seconds;
    int miliseconds = floor(value * 1000);

    printf("%02d:%02d:%02d,%03d", hours, minutes, seconds, miliseconds);
}

