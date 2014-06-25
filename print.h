#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#define ANSI_BLACK      "\33[30m"
#define ANSI_RED        "\33[31m"
#define ANSI_GREEN      "\33[32m"
#define ANSI_YELLOW     "\33[33m"
#define ANSI_BLUE       "\33[34m"
#define ANSI_MAGENTA    "\33[35m"
#define ANSI_CYAN       "\33[36m"
#define ANSI_WHITE      "\33[37m"
#define ANSI_BG_BLACK   "\33[40m"
#define ANSI_BG_RED     "\33[41m"
#define ANSI_BG_GREEN   "\33[42m"
#define ANSI_BG_YELLOW  "\33[43m"
#define ANSI_BG_BLUE    "\33[44m"
#define ANSI_BG_MAGENTA "\33[45m"
#define ANSI_BG_CYAN    "\33[46m"
#define ANSI_BG_WHITE   "\33[47m"
#define ANSI_BOLD       "\33[1m"
#define ANSI_ITALIC     "\33[3m"
#define ANSI_UNDERLINE  "\33[4m"
#define ANSI_BLINK      "\33[5m"
#define ANSI_NEGATIVE   "\33[7m"
#define ANSI_RESET      "\33[0m"

void print_indentation(int);

void fprint_units(FILE *, float);

void print_units(float);
void print_frequency(float);
void print_memory(float);
void print_time(float);
void print_log_time(float);
void print_array(char, void *, int);

int sprint_units(char *, float);
int sprintf_units(char *, char const *, float);
int sprint_frequency(char *, float);
int sprint_memory(char *, float);
int sprint_time(char *, float);
int sprintf_time(char *, char const *, float);
int sprint_time_classic(char *, float);

void fprint_date(FILE *);

#endif
