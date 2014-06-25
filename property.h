#ifndef PROPERTY_H
#define PROPERTY_H

typedef struct
{
    char const * key;
    void const * value;
}
Property;

typedef struct Property_Key
{
    int id;
    char const * name;
}
Property_Key;

typedef struct
{
    Property_Key const * keys;
    int count;
}
Property_Keys;

void property_destroy(Property *, int count);
void property_print(Property const properties[], int count);
void property_sort(Property properties[], int count);
char const * property_find(Property const properties[], int count, char const key[]);

int  property_parser(void const *, char const string[], void * target);
int  property_printer(void const *, void const * source, char string[]);

#endif
