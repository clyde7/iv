#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "memory.h"
#include "string.h"
#include "variable.h"

static char const * type_to_string(char type)
{
    switch (type)
    {
        default:  return "custom";
        case 'b': return "boolean";
        case 'B': return "boolean";
        case 'c': return "char";
        case 'd': return "int";
        case 'f': return "float";
        case 's': return "string";
    }

    return NULL;
}

void variable_print(Variable const * variable)
{
    printf("target = %lx\n", (unsigned long) variable->target);
    printf("type = %s\n", type_to_string(variable->type));
    printf("name = %s\n", variable->name);
    printf("short name = %s\n", variable->short_name);
    printf("description = %s\n", variable->description);
}

void variable_print_state(Variable const * variable)
{
    printf("%s = ", variable->name);

    switch (variable->type)
    {
        default:
            if (variable->extension)
            {
                char buffer[256];

                Variable_Extension const * extension = variable->extension;
                extension->print(extension->data, variable->target, buffer);

                printf("%s", buffer);
            }
            break;

        case 'b': printf("%d", * (int *)   variable->target); break;
        case 'B': printf("%d", * (int *)   variable->target); break;
        case 'c': printf("%c", * (char *)  variable->target); break;
        case 'd': printf("%d", * (int *)   variable->target); break;
        case 'f': printf("%g", * (float *) variable->target); break;
        case 's': printf("%s",   (char *)  variable->target); break;
    }

    puts("");
}

void variable_print_all(Variable const variables[], int count)
{
    for (int i = 0; i != count; ++ i)
    {
        Variable const * variable = &variables[i];
        variable_print_state(variable);
    }
}

int variable_parse(Variable const * variable, char const string[])
{
#if 0
    if (variable->parse)
    {
        variable->parse(variable, string);
        return 1;
    }
#endif

    switch (variable->type)
    {
        default:
        {
            Variable_Extension const * extension = variable->extension;
            return extension->parse(extension->data, string, (int *) variable->target);
        }
        case 'b': * (int *)   variable->target = atoi(string); break;
        case 'B': * (int *)   variable->target = atoi(string); break;
        case 'c': * (char *)  variable->target = string[0]; break;
        case 'd': * (int *)   variable->target = atoi(string); break;
        case 'f': * (float *) variable->target = atof(string); break;
        case 's': strcpy((char *) variable->target, string); break;
    }

    return 1;
}

int variable_sprint(char string[], Variable const * variable)
{
    switch (variable->type)
    {
        default:
        {
            Variable_Extension const * extension = variable->extension;
            return extension->print(extension->data, variable->target, string);
        }
        case 'b': return sprintf(string, "%d", * (int *)   variable->target);
        case 'B': return sprintf(string, "%d", * (int *)   variable->target);
        case 'c': return sprintf(string, "%d", * (char *)  variable->target);
        case 'd': return sprintf(string, "%d", * (int *)   variable->target);
        case 'f': return sprintf(string, "%g", * (float *) variable->target);
        case 's': return sprintf(string, "%s",   (char *)  variable->target);
    }
}

void variable_process_command(Variable const variables[], int count, char const command[])
{
    char name[256], value[256];

    sscanf(command, "%s = %s", name, value);

#ifdef DEBUG
    printf("command = \"%s\"\n", command);
    printf("name = \"%s\"\n", name);
    printf("value = \"%s\"\n", value);
#endif

    for (int i = 0; i != count; ++ i)
    {
        Variable const * variable = &variables[i];

        if (streq(variable->name, name))
        {
            variable_parse(variable, value);
            return;
        }
    }
}

static void replace(char string[])
{
    for (int i = 0; string[i]; ++ i)
    {
        if (string[i] == '_')
            string[i] = '-';
    }
}

int variable_parse_arguments(Variable const arguments[], int count, int argc, char * argv[])
{
    char name_buffer[256];

    int i, j;
    Variable const * default_argument = NULL;

    for (i = 1; i != argc; ++ i)
    {
        char const * arg = argv[i];
        Variable const * argument = NULL;

        for (j = 0; j != count; ++ j)
        {
            argument = &arguments[j];

            if (! argument->short_name) // && ! argument->long_name)
            {
                default_argument = argument;
                continue;
            }

            if (argument->name)
            {
                strcpy(name_buffer, "--");
                strcat(name_buffer, argument->name);
                replace(name_buffer);
            }

            if ((argument->short_name && streq(arg, argument->short_name)) ||
                (streq(arg, name_buffer)))
                //(argument->long_name && streq(arg, argument->long_name)))
                break;
        }

        if (j == count)
        {
            if (default_argument)
            {
                // TODO add use extension default argument handler
#if 1
                Variable_Extension const * extension = argument->extension;
                if (extension)
                {
                    extension->parse(extension->data, arg, argument->target);
                    continue;
                }
#endif
                * (char const **) default_argument->target = arg;
                continue;
            }
            else
            {
                fprintf(stderr, "unrecognized argument \"%s\"\n", arg);
                break;
            }
        }

        if (argument->type == 'h')
            return 0;

        if (argument->type == 'b')
        {
            * (int *) argument->target = 1;
            continue;
        }

        if (argument->type == 'B')
        {
            * (int *) argument->target = 0;
            continue;
        }

        if (i + 1 == argc)
            break;

        switch (argument->type)
        {
            case 'd':
                * (int *) argument->target = atoi(argv[++ i]);
                break;

            case 'f':
                * (float *) argument->target = atof(argv[++ i]);
                break;

            case 's':
                * (char const **) argument->target = argv[++ i];
                break;

#if 0
            case 'M':
                * (int *) argument->target = atom(argument->keys, argv[++ i]);
                break;
#endif

            default:
            {
                Variable_Extension const * extension = argument->extension;
                if (extension && extension->parse(extension->data, argv[++ i], (int *) argument->target))
                    break;
//                if (argument->parse && argument->parse(argument, argv[++ i])
//                    break;

                fprintf(stderr, "unknown argument type '%c'\n", argument->type);
                exit(EXIT_FAILURE);
            }
        }
    }

    return i == argc;
}

void variable_usage(Variable const arguments[], int count, char const name[])
{
    int i;

    fprintf(stderr, "usage:\n");
    fprintf(stderr, "\t%s [options] <%s>\n\n", name, "files");

    fprintf(stderr, "options:\n");

    for (i = 0; i != count; ++ i)
    {
        Variable const * argument = &arguments[i];

        if (! argument->name)
            continue;

        char long_name[256];
        strcpy(long_name, "--");
        strcat(long_name, argument->name);
        replace(long_name);

        if (argument->short_name)
        {
            fprintf(stderr, "\t%s, %s", argument->short_name, long_name);
        }
        else
        {
            fprintf(stderr, "\t%s", long_name);
        }

        switch (argument->type)
        {
            case 'b':
            case 'B':
            case 'h':
                fprintf(stderr, "\t%s\n", argument->description);
                break;

            default:
                fprintf(stderr, " <%s>", argument->description);

                switch (argument->type)
                {
                    case 's':
                        if (* (char **) argument->target)
                            fprintf(stderr, " (\"%s\")", * (char **) argument->target);
                        break;

                    case 'd': fprintf(stderr, " (%d)", * (int *) argument->target); break;
                    case 'f': fprintf(stderr, " (%.2g)", * (float *) argument->target); break;
                }

                puts("");
                break;
        }
    }
}

void argument_dump_2(int argc, char * argv[], char buffer[], int size)
{
    int index = 0;

    for (int i = 0; i != argc; ++ i)
    {
        index += sprintf(&buffer[index], "%s ", argv[i]);
    }
}

Variable * variable_merge(Variable const * arguments1, int count1, Variable const * arguments2, int count2)
{
    int new_count = count1 + count2;
    Variable * new_arguments = malloc_array(Variable, new_count);

    memcpy(&new_arguments[0], arguments1, count1 * sizeof(Variable));
    memcpy(&new_arguments[count1], arguments2, count2 * sizeof(Variable));

    return new_arguments;
}

Variable const * variable_find_name(Variable const variables[], int count, char const name[])
{
    for (int i = 0; i != count; ++ i)
    {
        if (streq(variables[i].name, name))
            return &variables[i];
    }

    return NULL;
}

Variable const * variable_find_key(Variable const variables[], int count, char key)
{
    for (int i = 0; i != count; ++ i)
    {
        if (variables[i].key == key)
            return &variables[i];
    }

    return NULL;
}

int name_parser(void const * data, char const string[], void * target_)
{
    list_append((List *) data, strdup(string));

    return 1;
}

