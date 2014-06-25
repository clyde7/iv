#ifndef STRING_H
#define STRING_H

#include <string.h>
#include <libgen.h>

#define streq(a, b) (strcmp((a), (b)) == 0)
#define strcaseeq(a, b) (strcasecmp((a), (b)) == 0)
#define starts_with(a, b) (strstr((a), (b)) == (a))

#if defined(CYGWIN)
char * strdup(char const string[]);
#endif

#ifdef WINDOWS
#define strdup _strdup
#endif

char const * basename_(char const *);
int find_string(char const * const strings[], int count, char const pattern[]);
void string_lower(char string[]);
void string_replace_character(char string[], char in, char out);
char * strndup_(char const string[], int count);
int string_ends_with(char const string[], char const suffix[]);

#endif
