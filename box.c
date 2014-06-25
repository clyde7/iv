#include <float.h>
#include <stdio.h>

#include "list.h"
#include "math_.h"
#include "memory.h"
#include "box.h"

Box const EMPTY_BOX =
{
    {0, 0, 0},
    {0, 0, 0}
};

Box const UNIT_BOX =
{
    {-1, -1, -1},
    {+1, +1, +1}
};

Box const ORIGIN_BOX =
{
    { 0,  0,  0},
    {+1, +1, +1}
};

Box const MIN_BOX =
{
    {+FLT_MAX, +FLT_MAX, +FLT_MAX},
    {-FLT_MAX, -FLT_MAX, -FLT_MAX}
};

Box const MAX_BOX =
{
    {-FLT_MAX, -FLT_MAX, -FLT_MAX},
    {+FLT_MAX, +FLT_MAX, +FLT_MAX}
};

Box const MONOLITH =
{
    {0, 0, 0},
    {1, 4, 9}
};

int box_parse(void const * data, char const string[], void * boxes_)
{
    int x, y, w, h;

    if (4 == sscanf(string, "%dx%d%d%d", &w, &h, &x, &y))
    {
    }
    else if (2 == sscanf(string, "%dx%d", &w, &h))
    {
        x = y = 0;
    }
    else if (2 == sscanf(string, "%d%d", &x, &y))
    {
        w = h = 1;
    }
    else
        return 0;

    Box * box = malloc_size(Box);
    box->min = vector(x, y, 0);
    box->max = vector(x + w, y + h, 0);

    box_print(*box); puts("");

    List * boxes = (List *) boxes_;
    list_append(boxes, box);

    return 1;
}

void box_print(Box box)
{
    printf("{");
    vector_print(box.min);
    printf("}, {");
    vector_print(box.max);
    printf("}");
}

Box box_fix(Box b)
{
    Box fixed_box =
    {
        {
            fmin(b.min.x, b.max.x),
            fmin(b.min.y, b.max.y),
            fmin(b.min.z, b.max.z)
        },
        {
            fmax(b.min.x, b.max.x),
            fmax(b.min.y, b.max.y),
            fmax(b.min.z, b.max.z)
        }
    };

    return fixed_box;
}

Box box_union(Box b1, Box b2)
{
    if (box_empty(b1))
        return b2;

    if (box_empty(b2))
        return b1;

    Box b =
    {
        vector_min(b1.min, b2.min),
        vector_max(b1.max, b2.max)
    };

    return b;
}

Box box_add(Box b, Vector p)
{
    b.min = vector_min(b.min, p);
    b.max = vector_max(b.max, p);

    return b;
}

Box box_translate(Box b, Vector p)
{
    b.min = vector_add(b.min, p);
    b.max = vector_add(b.max, p);

    return b;
}

int box_empty(Box b)
{
    return
        b.min.x >= b.max.x ||
        b.min.y >= b.max.y ||
        b.min.z >= b.max.z;
}

int box_overlap(Box b1, Box b2)
{
    return
        (b1.max.x >= b2.min.x) && (b1.min.x <= b2.max.x) &&
        (b1.max.y >= b2.min.y) && (b1.min.y <= b2.max.y) &&
        (b1.max.z >= b2.min.z) && (b1.min.z <= b2.max.z);
}

int box_inside(Box b, Vector p)
{
    return
        (p.x >= b.min.x) && (p.x <= b.max.x) &&
        (p.y >= b.min.y) && (p.y <= b.max.y) &&
        (p.z >= b.min.z) && (p.z <= b.max.z);
}

Box box_expand(Box b, float delta)
{
    b.min.x -= delta;
    b.min.y -= delta;
    b.min.z -= delta;

    b.max.x += delta;
    b.max.y += delta;
    b.max.z += delta;

    return b;
}

Box box_scale(Box b, float delta)
{
    Vector center, corner;
    box_bounding_sphere(b, &center, &corner);

    corner = vector_scale(corner, delta);        

    return box_from_bounding_sphere(center, corner);
}

Box box_transform(Box b, Matrix matrix)
{
    Vector corners[8];
    box_to_vertices(b, corners);

    Box box = EMPTY_BOX;
    for (int i = 0; i != 8; ++ i)
    {
        Vector v = matrix_mul(matrix, corners[i]);
        box = box_add(box, v);
    }

    return box;
}

float box_volume(Box b)
{
    Vector d = vector_sub(b.max, b.min);
    return d.x * d.y * d.z;
}

int box_max_extent(Box b)
{
    Vector d = vector_sub(b.max, b.min);

    if (d.x >= d.y)
        return (d.x >= d.z) ? 0 : 2;
    else
        return (d.y >= d.z) ? 1 : 2;
}

Vector box_center(Box b)
{
    return vector_scale(vector_add(b.max, b.min), 0.5);
}

void box_bounding_sphere(Box b, Vector * center, Vector * corner)
{
    *center = vector_scale(vector_add(b.max, b.min), 0.5);
    *corner = vector_scale(vector_sub(b.max, b.min), 0.5);
}

Box box_from_bounding_sphere(Vector center, Vector corner)
{
    Box box =
    {
        vector_sub(center, corner),
        vector_add(center, corner)
    };

    return box;
}

Vector box_size(Box b)
{
    return vector_sub(b.max, b.min);
}

Vector box_ratio(Box box)
{
    Vector size = box_size(box);
    float const max_size = fmax(fmax(size.x, size.y), size.z);

    return vector_scale(size, 1.0 / max_size);
}

Box box_from_vertices(Vector const vertices[], int count)
{
    Box box = MIN_BOX;

    for (int i = 0; i != count; ++ i)
    {
        Vector p = vertices[i];
        box.min = vector_min(box.min, p);
        box.max = vector_max(box.max, p);
    }

    return box;
}

Vector vector_random(Box box)
{
    Vector v =
    {
        random_range(box.min.x, box.max.x),
        random_range(box.min.y, box.max.y),
        random_range(box.min.z, box.max.z)
    };

    return v;
}

void box_to_vertices(Box b, Vector vertices[8])
{
    vertices[0] = vector(b.min.x, b.min.y, b.min.z);
    vertices[1] = vector(b.max.x, b.min.y, b.min.z);
    vertices[2] = vector(b.min.x, b.max.y, b.min.z);
    vertices[3] = vector(b.max.x, b.max.y, b.min.z);
    vertices[4] = vector(b.min.x, b.min.y, b.max.z);
    vertices[5] = vector(b.max.x, b.min.y, b.max.z);
    vertices[6] = vector(b.min.x, b.max.y, b.max.z);
    vertices[7] = vector(b.max.x, b.max.y, b.max.z);
}

float box_surface_area(Box b)
{
    Vector size = box_size(b);
    return 2.0 * (size.x * size.y + size.y * size.z + size.z * size.x);
}

float box_area(Box b)
{
    Vector size = box_size(b);
    return size.x * size.y;
}

Vector box_interpolate_point(Box box, Vector v)
{
    Vector w =
    {
        interpolate(box.min.x, box.max.x, v.x),
        interpolate(box.min.y, box.max.y, v.y),
        interpolate(box.min.z, box.max.z, v.z)
    };

    return w;
}

