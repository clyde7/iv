#include <stdlib.h>

#include "list.h"
#include "math_.h"
#include "memory.h"

List const EMPTY_LIST = {NULL, 0, 0};

void list_append(List * list, void * entry)
{
    if (list->count == list->size)
    {
        list->size = (list->size == 0) ? 1 : list->size * 2;
        list->entries = realloc_array(void *, list->entries, list->size);
    }

    list->entries[list->count ++] = entry;
}

void * list_remove(List * list)
{
    return list->entries[-- list->count];
}

void list_fill(List * list, void ** entries, int count)
{
    if (list->count + count > list->size)
    {
        list->size = (list->size == 0) ? count : imax(list->size * 2, list->size + count);
        list->entries = realloc_array(void *, list->entries, list->size);
    }

    memcpy(&list->entries[list->count], entries, count * sizeof(void *));
    list->count += count;
}

void list_clear(List * list)
{
    free(list->entries);
    list->entries = NULL;
    list->size = list->count = 0;
}

void list_sort(Sortable_List * list)
{
    qsort(list->list.entries, list->list.count, sizeof(void *), list->compare);
}

void * list_find(Sortable_List const * list, void const * key)
{
    return bsearch(key, list->list.entries, list->list.count, sizeof(void *), list->compare);
}

void list_copy(List const * source, List * target)
{
    for (int i = 0; i != source->count; ++ i)
    {
        void * element = source->entries[i];
        list_append(target, element);
    }
}

void list_print(List const * list, void (* print)(void const *))
{
    for (int i = 0; i != list->count; ++ i)
    {
        void const * element = list->entries[i];
        print(element);
    }
}
