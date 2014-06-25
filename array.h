#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>

#include "box.h"
#include "color.h"
#include "opengl.h"
#include "vector.h"

typedef struct {unsigned a, b;} Pair;
typedef struct {unsigned a, b, c;} Face;
typedef struct {unsigned a, b, c, d;} Quad;
typedef struct {unsigned a, b, c, d, e;} Pentagon;

typedef struct
{
    GLenum mode;
    GLuint buffer[5];
    int vertex_count, face_count;
    Vector *vertices, *normals, *tex_coords;
    Color *colors;
    union
    {
        int  *indices;
        Pair *pairs;
        Face *triangles;
        Quad *quads;
    }
    faces;
}
Array;

typedef Array * (Array_Factory) (int);

extern Array const EMPTY_ARRAY;


Array * array_new(GLenum mode); // TODO replace mallocs
Box  array_box(Array const *);
void array_print_info(Array const *);
void array_print(Array const *);
#define array_draw(array) array_draw_as((array), (array)->mode)
void array_draw_as(Array const *, GLenum mode);
void array_draw_slow(Array const *);
void array_draw_slow_as(Array const *, GLenum mode);
void array_draw_fast(Array const *);
void array_draw_fast_as(Array const *, GLenum mode);
void array_clear(Array *);
void array_destroy(Array *);
void array_resize(Array *, Box);
void array_add_normals(Array *);
void array_add_indices(Array *);
void array_interpolate(Array const * from, Array const * to, float t, Array * target);
Array * array_copy(Array const *);
Array * array_open(char const name[]);
int  array_append_vertex(Array *, Vector);
int  array_append_point(Array *, Vector);
void array_append_line(Array *, Vector, Vector);
void array_append_triangle(Array *, Vector, Vector, Vector);
void array_append_quad(Array *, Vector, Vector, Vector, Vector);
void array_append_quad_indices(Array *, int, int, int, int);
//int  array_append_vertex_2(Array *, Vector vertex, Vector normal, Vector tex_coord);
void array_flip_normals(Array *);

Array * array_flatten_bezier(Array const *, int count);
Array * array_flatten_indices(Array const *);
void array_draw_as_bezier(Array const *, int count);

Array * array_to_triangles(Array const *);

#endif
