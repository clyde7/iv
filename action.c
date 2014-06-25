#include <stdlib.h>
#include <pthread.h>

#include "action.h"
#include "math_.h"

#define LOCK(mutex)   pthread_mutex_lock(&mutex)
#define UNLOCK(mutex) pthread_mutex_unlock(&mutex)

typedef struct {Action action; void * data;} Entry;

static pthread_mutex_t mutex;
static Entry * entries;
static unsigned size, count;

void action_initialize(void)
{
    static int initialized;
    if (initialized)
        return;

    pthread_mutex_init(&mutex, NULL);
    initialized = 1;
}

int action_count(void)
{
    return count;
}

void action_add(Action action, void * data)
{
    LOCK(mutex);

    if (count == size)
    {
        size = imax(size * 2, 1);
        entries = (Entry *) realloc(entries, size * sizeof(Entry));
    }

    entries[count].action = action;
    entries[count].data = data;

    ++ count;
    
    UNLOCK(mutex);
}

void action_process(void)
{
    Entry * local_entries;
    int local_count, i;

    LOCK(mutex);

    if (!entries)
    {
        UNLOCK(mutex);
        return;
    }

    local_entries = entries;
    local_count = count;

    entries = NULL;
    count = 0;
    size = 0;

    UNLOCK(mutex);

    for (i = 0; i != local_count; ++ i)
    {
        Action action = local_entries[i].action;
        void * data = local_entries[i].data;

        if (action(data))
        {
            action_add(action, data);
        }
        else
        {
            free(data);
        }
    }

    free(local_entries);
}
