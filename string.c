#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "string.h"

char const * basename_(char const name[])
{
    char const * last = strrchr(name, '/');
    return last ? last + 1 : name;
}

#if defined(CYGWIN)
char * strdup(char const string[])
{
    char * new_string = (char *) malloc(strlen(string) + 1);
    strcpy(new_string, string);
    return new_string;
}
#endif

int find_string(char const * const strings[], int count, char const pattern[])
{   
    int i;

    for (i = 0; i != count; ++ i)
    {   
        if (strstr(pattern, strings[i]))
            return 1;
    }

    return 0;
}

void string_lower(char string[])
{
    int const length = strlen(string);
    int i;

    for (i = 0; i != length; ++ i)
    {
        string[i] = tolower((unsigned char) string[i]);
    }
}

void string_replace_character(char string[], char in, char out)
{
    int const length = strlen(string);
    int i;

    for (i = 0; i != length; ++ i)
    {
        if (string[i] == in)
            string[i] = out;
    }
}

char * strndup_(char const string[], int count)
{
    char * new_string = (char *) malloc(count + 1);
    strncpy(new_string, string, count);

    new_string[count] = '\0';

    return new_string;
}

int string_ends_with(char const string[], char const suffix[])
{
    int string_length = strlen(string);
    int suffix_length = strlen(suffix);

    if (string_length < suffix_length)
        return 0;

    return 0 == strcmp(&string[string_length - suffix_length], suffix);
}

