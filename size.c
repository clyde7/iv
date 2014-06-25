#include <limits.h>
#include <stdio.h>

#include "error.h"
#include "size.h"
#include "viewport.h"

Size const
    MIN_SIZE = {INT_MIN, INT_MIN, INT_MIN},
    MAX_SIZE = {INT_MAX, INT_MAX, INT_MAX}; 

Size size_wrap(int x, int y, int z)
{
    Size size = {x, y, z};
    return size;
}

void size_print(Size size)
{
    printf("%d, %d, %d", size.x, size.y, size.z);
}

int size_volume(Size size)
{
    return size.x * size.y * size.z;
}

int size_empty(Size size)
{
    return 
        size.x == 0 ||
        size.y == 0 ||
        size.z == 0;
}

Size size_stride(Size size)
{
    Size stride = {1, size.x, size.x * size.y};
    return stride;
}

Size size_add(Size a, Size b)
{
    Size s = {a.x + b.x, a.y + b.y, a.z + b.z};
    return s;
}

Size size_scale(Size a, Size b)
{
    Size s = {a.x*b.x, a.y*b.y, a.z*b.z};
    return s;
}

int size_index(Size size, int i, int j, int k)
{
    return k + j * size.x + i * size.x * size.y;
}

int size_total(Size size)
{
    return size.x * size.y * size.z;
}

int size_equal(Size a, Size b)
{
    return
        a.x == b.x &&
        a.y == b.y &&
        a.z == b.z;
}

Size size_min(Size a, Size b)
{
    Size size =
    {
        imin(a.x, b.x),
        imin(a.y, b.y),
        imin(a.z, b.z)
    };

    return size;
}

Size size_max(Size a, Size b)
{
    Size size =
    {
        imax(a.x, b.x),
        imax(a.y, b.y),
        imax(a.z, b.z)
    };

    return size;
}

#if 0
int size_parser(void const * data, char const string[], void * target_)
{
    int * target = (int *) target_;

    return
        resolution_from_name(string, (Resolution *) target) ||
        sscanf(string, "%dx%d", &((int *) target)[0], &((int *) target)[1]) == 2;
}
#else
int size_parser(void const * data, char const string[], void * target_)
{
    Size * size = (Size *) target_;

    Resolution resolution;
    if (resolution_from_name(string, &resolution))
    {
        size->x = resolution.width;
        size->y = resolution.height;
        size->z = 1;

        return 1;
    }

    if (sscanf(string, "%dx%dx%d", &size->x, &size->y, &size->z) == 3)
        return 1;

    if (sscanf(string, "%dx%d", &size->x, &size->y) == 2)
    {
        size->z = 1;
        return 1;
    }

    if (sscanf(string, "%d", &size->x) == 1)
    {
        size->y = size->z = 1;
        return 1;
    }

    return 0;
}

int tuple_parser(void const * data, char const string[], void * target_)
{
    Size * size = (Size *) target_;
    Size default_size = * size;

    if (sscanf(string, "%dx%dx%d", &size->x, &size->y, &size->z) == 3)
        return 1;

    if (sscanf(string, "%dx%d", &size->x, &size->y) == 2)
    {
        size->z = default_size.z == -1 ? size->y : default_size.z;
        return 1;
    }

    if (sscanf(string, "%d", &size->x) == 1)
    {
        size->y = default_size.y == -1 ? size->x : default_size.y;
        size->z = default_size.z == -1 ? size->x : default_size.z;
        return 1;
    }

    return 0;
}

#endif

int size_printer(void const * data, void const * source, char string[])
{
    Size size = * (Size *) source;
    return sprintf(string, "%dx%d", size.x, size.y);
}

int size_index_border(Size size, int i, int j, int k, Border border)
{
    switch (border)
    {
        case BORDER_CLAMP:
            i = iclamp_to(i, 0, size.z - 1);
            j = iclamp_to(j, 0, size.y - 1);
            k = iclamp_to(k, 0, size.x - 1);
            break;

        case BORDER_MIRROR:
            if (i < 0) i = - i - 1; else if (i >= size.z) i = 2 * size.z - i - 1;
            if (j < 0) j = - j - 1; else if (j >= size.y) j = 2 * size.y - j - 1;
            if (k < 0) k = - k - 1; else if (k >= size.x) k = 2 * size.x - k - 1;
            break;

        case BORDER_WRAP:
            while (i < 0) i += size.z; while (i >= size.z) i -= size.z;
            while (j < 0) j += size.y; while (j >= size.y) j -= size.y;
            while (k < 0) k += size.x; while (k >= size.x) k -= size.x;
            break;

        default:
            error_fail("unsupported border");
            break;
    }

    return size_index(size, i, j, k);
}

