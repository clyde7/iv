#include <stdlib.h>
#ifndef WINDOWS
#include <unistd.h>
#endif

#ifdef CYGWIN
extern int mkstemp(char *);
#endif

#include "file.h"
#include "memory.h"

char * file_read_text(char const name[], unsigned * length)
{
    FILE * const file = fopen(name, "rt");
    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    int count = ftell(file);
    if (count == 0)
    {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char * const content = (char *) malloc(sizeof(char) * (count + 1));
    count = fread(content, sizeof(char), count, file);
    content[count] = '\0';

    if (length)
        *length = count;

    fclose(file);
    return content;
}

void * file_read_binary(char const name[], unsigned * length)
{
    FILE * const file = fopen(name, "rb");
    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    int count = ftell(file);
    if (count == 0)
    {
        fclose(file);
        return NULL;
    }

    rewind(file);

    void * const content = malloc(count);
    count = fread(content, sizeof(char), count, file);

    if (length)
        *length = count;

    fclose(file);
    return content;
}

void file_unique_name(char buffer[], char const pattern[])
{
    FILE * file = NULL;
    int i = 0;

    do
    {
        if (file)
            fclose(file);
        sprintf(buffer, pattern, i ++);
        file = fopen(buffer, "rb");
    }
    while (file != NULL);
}

void file_temporary_name(char buffer[])
{
#ifdef WINDOWS
    tmpnam(buffer);
#else
    static char const pattern[] = "cuboid-XXXX.tmp";
    strcpy(buffer, pattern);
    mkstemp(buffer);
#endif
}

FILE * file_open_unique(char const pattern[])
{
    char buffer[256];
    file_unique_name(buffer, pattern);

    return fopen(buffer, "wb");
}

static void strip_trailing_new_line(char string[])
{
    int count = strlen(string);

    if (count && string[count - 1] == '\n')
        string[-- count] = '\0';

    if (count && string[count - 1] == '\r')
        string[-- count] = '\0';
}

char ** file_read_text_lines(char const name[], unsigned * line_count)
{
    FILE * file = fopen(name, "r");
    char * line;
    char ** lines = NULL;
    int count = 0;

    char buffer[512];

    while ((line = fgets(buffer, 512, file)))
    {
        lines = realloc_array(char *, lines, count + 1);
        strip_trailing_new_line(line);
        lines[count ++] = strdup(line);
    }

    fclose(file);

    * line_count = count;
    return lines;
}

