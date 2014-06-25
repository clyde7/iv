#ifndef LIST_H
#define LIST_H

typedef struct {void ** entries; int size, count;} List;
typedef int (* Comparator) (void const *, void const *);
typedef struct {List list; Comparator compare;} Sortable_List;

extern List const EMPTY_LIST;

void   list_append(List *, void *);
void * list_remove(List *);
void   list_fill(List *, void **, int);
void   list_clear(List *);
void   list_sort(Sortable_List *);
void * list_find(Sortable_List const *, void const * key);
void   list_copy(List const *, List *);
void   list_print(List const *, void (*) (void const *));

#endif
