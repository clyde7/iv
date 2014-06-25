#include "math_.h"
#include "memory.h"

void memory_randomize(void * data, int size)
{
    unsigned char * bytes = (unsigned char *) data;

    for (int i = 0; i != size; ++ i)
    {
        bytes[i] = floor(random2() * 255);
    }
}

void * copy_array_(void const * source, int size)
{
    void * target = malloc(size);
    memcpy(target, source, size);
    return target;
}

