#include <stdio.h>
#include <stdlib.h>

#include "property.h"
#include "string.h"

void property_destroy(Property * properties, int count)
{
    for (int i = 0; i != count; ++ i)
    {
        free((unsigned char *) properties[i].key);
        free((unsigned char *) properties[i].value);
    }

    if (properties)
        free(properties);
}

void property_print(Property const properties[], int count)
{
    for (int i = 0; i != count; ++ i)
    {
        printf("%s = %s\n", properties[i].key, (char const *) properties[i].value);
    }
}

static int property_comparator(void const * data_1, void const * data_2)
{
    Property const * property_1 = (Property const *) data_1;
    Property const * property_2 = (Property const *) data_2;

    return strcmp(property_1->key, property_2->key);
}

void property_sort(Property properties[], int count)
{
    qsort(properties, count, sizeof(Property), property_comparator);
}

char const * property_find(Property const properties[], int count, char const key[])
{
    Property reference = {key, NULL};
    Property * property = (Property *) bsearch(&reference, properties, count, sizeof(Property), property_comparator);

    return property ? (char const *) property->value : NULL;
}

int property_parser(void const * data, char const string[], void * target)
{
    Property_Keys const * keys = (Property_Keys const *) data;
    int * id = (int *) target;

    for (int i = 0; i != keys->count; ++ i)
    {
        Property_Key const * key = &keys->keys[i];
        if (strcaseeq(key->name, string))
        {
            *id = key->id;
            return 1;
        }
    }

    return 0;
}

int property_printer(void const * data, void const * source, char string[])
{
    Property_Keys const * keys = (Property_Keys const *) data;
    int id = * (int const *) source;

    for (int i = 0; i != keys->count; ++ i)
    {
        Property_Key const * key = &keys->keys[i];
        if (key->id == id)
        {
            strcpy(string, key->name);
            return 1;
        }
    }

    return 0;
}
