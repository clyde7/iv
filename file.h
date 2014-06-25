#ifndef FILE_H
#define FILE_H

#include <stdio.h>

char *  file_read_text(char const name[], unsigned * length);
char ** file_read_text_lines(char const name[], unsigned * line_count);
void *  file_read_binary(char const name[], unsigned * length);
FILE *  file_open_unique(char const pattern[]);
void    file_unique_name(char buffer[], char const pattern[]);
void    file_temporary_name(char buffer[]);

#endif
