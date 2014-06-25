#ifndef VARIABLE_H
#define VARIABLE_H

#define NIL ('\0')

struct Variable;

typedef struct
{
    void const * data;
    int (* parse)(void const * data, char const *, void * target);
    int (* print)(void const * data, void const * source, char *);
}
Variable_Extension;

typedef struct Variable
{
    void *target;
    char type, key;
    char const *name, *short_name, *description;
    Variable_Extension const * extension;
}
Variable;

void variable_print(Variable const *);
int  variable_parse(Variable const *, char const string[]);
int  variable_sprint(char string[], Variable const *);
void variable_print_all(Variable const variables[], int count);
void variable_process_command(Variable const variables[], int count, char const command[]);
int  variable_parse_arguments(Variable const variables[], int count, int argc, char * argv[]);

Variable * variable_merge(Variable const * arguments1, int count1, Variable const * arguments2, int count2);
void variable_usage(Variable const arguments[], int count, char const name[]);

Variable const * variable_find_name(Variable const variables[], int count, char const name[]);
Variable const * variable_find_key(Variable const variables[], int count, char key);

void argument_dump_2(int argc, char * argv[], char buffer[], int size); // rename

int name_parser(void const * data, char const string[], void * target_);

#endif
