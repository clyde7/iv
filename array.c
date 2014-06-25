#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "error.h"
#include "math_.h"
#include "memory.h"
#include "opengl.h"

typedef enum { VERTEX, NORMAL, TEX_COORD, COLOR, INDEX } Buffer_Type;

Array const EMPTY_ARRAY =
{
    GL_TRIANGLES, 
    {0, 0, 0, 0, 0},
    0, 0,
    NULL, NULL, NULL, NULL, 
    {NULL}
};

static char const * mode_to_string(GLenum mode)
{
    switch (mode)
    {
        default:
        case GL_POINTS:         return "points";
        case GL_LINES:          return "lines";
        case GL_LINE_STRIP:     return "line strip";
        case GL_LINE_LOOP:      return "line loop";
        case GL_TRIANGLES:      return "triangles";
        case GL_TRIANGLE_STRIP: return "triangle strip";
        case GL_TRIANGLE_FAN:   return "triangle fan";
        case GL_QUADS:          return "quads";
        case GL_QUAD_STRIP:     return "quad strip";
        case GL_POLYGON:        return "polygon";
    }
}

static int array_index_count(Array const * array)
{
    int face_count = array->face_count;

    switch (array->mode)
    {
        default:
        case GL_POINTS:
            return face_count;

        case GL_LINES:
            return face_count * 2;

        case GL_LINE_STRIP:
            return face_count + 1;

        case GL_LINE_LOOP:
            return face_count;

        case GL_TRIANGLES:
            return face_count * 3;

        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_POLYGON:
            return face_count + 2;

        case GL_QUADS:
            return face_count * 4;

        case GL_QUAD_STRIP:
            return face_count * 2 + 2;
    }
}

void array_print_info(Array const * array)
{
    printf("size = "); vector_print(box_size(array_box(array)));
    printf(", %d vertices, %d %s, ", array->vertex_count, array->face_count, mode_to_string(array->mode)); // improve string output
    printf( "%s normals, %s colors\n", array->normals ? "has" : "no", array->colors ? "has" : "no");
}

void array_print(Array const * array)
{
    int i;

    array_print_info(array);

    int vertex_count = array->vertex_count;

    printf("vertices =\n");
    for (i = 0; i != vertex_count; ++ i)
    {
        if (i != 0)
            printf(",\n");
        printf("{");
        vector_print(array->vertices[i]);
        printf("}");
    }
    puts("");

    if (array->colors)
    {
        printf("colors =\n");
        for (i = 0; i != vertex_count; ++ i)
        {
            if (i != 0)
                printf(",\n");
            printf("{");
            color_print(array->colors[i]);
            printf("}");
        }
        puts("");
    }

    if (array->tex_coords)
    {
        printf("texture coords =\n");
        for (i = 0; i != vertex_count; ++ i)
        {
            if (i != 0)
                printf(",\n");
            printf("{");
            vector_print(array->tex_coords[i]);
            printf("}");
        }
        puts("");
    }

    if (array->normals)
    {
        printf("normals =\n");
        for (i = 0; i != vertex_count; ++ i)
        {
            if (i != 0)
                printf(",\n");
            printf("{");
            vector_print(array->normals[i]);
            printf("}");
        }
        puts("");
    }

    printf("indices =\n");
    int index_count = array_index_count(array);
    for (i = 0; i != index_count; ++ i)
    {
        if (i != 0)
            printf(", ");
        printf("%d", array->faces.indices[i]);
    }
    puts("");
}

void array_draw_as(Array const * array, GLenum mode)
{
    if (array->colors)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, 0, array->colors);
    }

    if (array->tex_coords)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(3, GL_FLOAT, 0, array->tex_coords);
    }

    if (array->normals)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, array->normals);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, array->vertices);

    array->face_count
        ? glDrawElements(mode, array_index_count(array), GL_UNSIGNED_INT, array->faces.indices)
        : glDrawArrays(mode, 0, array->vertex_count);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void array_draw_slow_as(Array const * array, GLenum mode)
{
    int const index_count = array_index_count(array);
    int const * indices = array->faces.indices;
    int  i;

    glBegin(mode);

    if (array->colors)
    {
        if (array->normals)
        {
            for (i = 0; i != index_count; ++ i)
            {
                GLuint index = indices[i];

                glColor4fv((GLfloat const *) &array->colors[index]);
//                glTexCoord3fv((GLfloat const *) &array->tex_coords[index]);
                glNormal3fv((GLfloat const *) &array->normals[index]);
                glVertex3fv((GLfloat const *) &array->vertices[index]);
            }
        }
        else
        {
            for (i = 0; i != index_count; ++ i)
            {
                GLuint index = indices[i];

                glColor4fv((GLfloat const *) &array->colors[index]);
//                glTexCoord3fv((GLfloat const *) &array->tex_coords[index]);
                glVertex3fv((GLfloat const *) &array->vertices[index]);
            }
        }
    }
    else
    {
        if (array->normals)
        {
            for (i = 0; i != index_count; ++ i)
            {
                GLuint index = indices[i];

//                glTexCoord3fv((GLfloat const *) &array->tex_coords[index]);
                glNormal3fv((GLfloat const *) &array->normals[index]);
                glVertex3fv((GLfloat const *) &array->vertices[index]);
            }
        }
        else
        {
            for (i = 0; i != index_count; ++ i)
            {
                GLuint index = indices[i];

//                glTexCoord3fv((GLfloat const *) &array->tex_coords[index]);
                glVertex3fv((GLfloat const *) &array->vertices[index]);
            }
        }
    }

    glEnd();
}

void array_draw_slow(Array const * array)
{
    array_draw_slow_as(array, array->mode);
}

static void download_buffer(Array const * array)
{
    int vertex_count = array->vertex_count;

    //GLenum draw_mode = GL_STATIC_DRAW;
    GLenum draw_mode = GL_DYNAMIC_DRAW;

    if (array->colors)
    {
        glBindBuffer(GL_ARRAY_BUFFER, array->buffer[COLOR]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * 4 * sizeof(GLfloat), array->colors, draw_mode);
        glColorPointer(4, GL_FLOAT, 0, NULL);
    }

    if (array->normals)
    {
        glBindBuffer(GL_ARRAY_BUFFER, array->buffer[NORMAL]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(GLfloat), array->normals, draw_mode);
        glNormalPointer(GL_FLOAT, 0, NULL);
    }

    if (array->tex_coords)
    {
        glBindBuffer(GL_ARRAY_BUFFER, array->buffer[TEX_COORD]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(GLfloat), array->tex_coords, draw_mode);
        glTexCoordPointer(3, GL_FLOAT, 0, NULL);
    }

    if (array->faces.indices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, array->buffer[INDEX]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, array_index_count(array) * sizeof(GLuint), array->faces.indices, draw_mode);
    }

    glBindBuffer(GL_ARRAY_BUFFER, array->buffer[VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(GLfloat), array->vertices, draw_mode);
}

void array_draw_fast_as(Array const * array, GLenum mode)
{
    if (! array->buffer[0])
    {
        glGenBuffers(5, (GLuint *) array->buffer);

        download_buffer(array);
    }

    if (array->colors)
    {
        glBindBuffer(GL_ARRAY_BUFFER, array->buffer[COLOR]);
        glColorPointer(4, GL_FLOAT, 0, NULL);
        glEnableClientState(GL_COLOR_ARRAY);
    }

    if (array->tex_coords)
    {
        glBindBuffer(GL_ARRAY_BUFFER, array->buffer[TEX_COORD]);
        glTexCoordPointer(3, GL_FLOAT, 0, NULL);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (array->normals)
    {
        glBindBuffer(GL_ARRAY_BUFFER, array->buffer[NORMAL]);
        glNormalPointer(GL_FLOAT, 0, NULL);
        glEnableClientState(GL_NORMAL_ARRAY);
    }

    glBindBuffer(GL_ARRAY_BUFFER, array->buffer[VERTEX]);
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    glEnableClientState(GL_VERTEX_ARRAY);

    if (array->faces.indices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, array->buffer[INDEX]);
        glDrawElements(mode, array_index_count(array), GL_UNSIGNED_INT, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
        glDrawArrays(mode, 0, array->vertex_count);
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);  

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void array_draw_fast(Array const * array)
{
    array_draw_fast_as(array, array->mode);
}

void array_clear(Array * array)
{
    free(array->vertices);
    free(array->normals);
    free(array->colors);
    free(array->faces.indices);

    array->vertices = NULL;
    array->normals = NULL;
    array->tex_coords = NULL;
    array->colors = NULL;
    array->faces.indices = NULL;

    array->vertex_count = 0;
    array->face_count = 0;
}

void array_destroy(Array * array)
{
    if (! array)
        return;

    free(array->vertices);
    free(array->normals);
    free(array->tex_coords);
    free(array->colors);
    free(array->faces.indices);
    free(array);
}

void array_resize(Array * array, Box box)
{
    Vector center0, center1, corner0, corner1;

    box_bounding_sphere(array_box(array), &center0, &corner0);
    box_bounding_sphere(box, &center1, &corner1);

    Vector factor = vector_div(corner1, corner0);

    /* TODO what would be more elegant? */
    if (corner0.x == 0 && corner1.x == 0)
        factor.x = 1;
    if (corner0.y == 0 && corner1.y == 0)
        factor.y = 1;
    if (corner0.z == 0 && corner1.z == 0)
        factor.z = 1;

    int i;
    for (i = 0; i != array->vertex_count; ++ i)
    {
        Vector vertex = array->vertices[i];
        array->vertices[i] = vector_add(vector_mul(vector_sub(vertex, center0), factor), center1);
    }
}

void array_add_normals(Array * array)
{
    int i;

    assert(! array->normals);

    int face_count = array->face_count;
    int vertex_count = array->vertex_count;

    Vector * vertices = array->vertices;
    Vector * normals = calloc_array(Vector, vertex_count);

    switch (array->mode)
    {
        case GL_TRIANGLES:
            {
                Face * faces = array->faces.triangles;

                if (faces)
                for (i = 0; i != face_count; ++ i)
                {
                    Face * triangle = &faces[i];
                    int a = triangle->a;
                    int b = triangle->b;
                    int c = triangle->c;

                    Vector normal = triangle_normal(vertices[a], vertices[b], vertices[c]);

                    normals[a] = vector_add(normals[a], normal);
                    normals[b] = vector_add(normals[b], normal);
                    normals[c] = vector_add(normals[c], normal);
                }
                else
                {
                    // untested!
                    for (i = 0; i != vertex_count; i += 3)
                    {
                         Vector normal = triangle_normal(vertices[i + 0], vertices[i + 1], vertices[i + 2]);

                         normals[i + 0] = vector_add(normals[i + 0], normal);
                         normals[i + 1] = vector_add(normals[i + 1], normal);
                         normals[i + 2] = vector_add(normals[i + 2], normal);
                    }
                }
            }
            break;

        case GL_QUADS:
            {
                Quad * faces = array->faces.quads;

                if (faces)
                for (i = 0; i != face_count; ++ i)
                {
                    Quad * quad = &faces[i];
                    int a = quad->a;
                    int b = quad->b;
                    int c = quad->c;
                    int d = quad->d;

                    Vector normal = triangle_normal(vertices[a], vertices[b], vertices[c]);
                    if (isnan(normal.x) || isnan(normal.y) || isnan(normal.z))
                        normal = triangle_normal(vertices[a], vertices[c], vertices[d]);

                    normals[a] = vector_add(normals[a], normal);
                    normals[b] = vector_add(normals[b], normal);
                    normals[c] = vector_add(normals[c], normal);
                    normals[d] = vector_add(normals[d], normal);
                }
                else
                {
                    for (i = 0; i != vertex_count; i += 4)
                    {
#if 0
                         Vector normal1 = triangle_normal(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
                         Vector normal2 = triangle_normal(vertices[i + 1], vertices[i + 2], vertices[i + 3]);
                         Vector normal3 = triangle_normal(vertices[i + 2], vertices[i + 3], vertices[i + 0]);
                         Vector normal4 = triangle_normal(vertices[i + 3], vertices[i + 0], vertices[i + 1]);
                         Vector normal = vector_normalize(vector_add4(normal1, normal2, normal3, normal4));
#else
                         Vector a = vector_sub(vertices[i + 0], vertices[i + 2]);
                         Vector b = vector_sub(vertices[i + 1], vertices[i + 3]);
                         Vector normal = vector_normal(a, b);
#endif

                         normals[i + 0] =
                         normals[i + 1] =
                         normals[i + 2] =
                         normals[i + 3] = normal;
                    }
                }
            }
            break;
    }

    for (i = 0; i != vertex_count; ++ i)
    {
        normals[i] = vector_normalize(normals[i]);
    }

    array->normals = normals;
}

static int mode_to_vertices(GLenum mode)
{
    switch (mode)
    {
        case GL_POINTS:    return 1;
        case GL_LINES:     return 2;
        case GL_TRIANGLES: return 3;
        case GL_QUADS:     return 4;
    }

    return -1;
}

void array_add_indices(Array * array)
{
    assert(! array->faces.indices);

    int const vertices_per_face = mode_to_vertices(array->mode);
    error_check(vertices_per_face == -1, "not a simple array");

    int const index_count = array->vertex_count;
    int i;

    int * indices = malloc_array(int, index_count);
    for (i = 0; i != index_count; ++ i)
    {
        indices[i] = i;
    }

    array->face_count = index_count / vertices_per_face;
    array->faces.indices = indices;
}

void array_interpolate(Array const * array1, Array const * array2, float t, Array * array3)
{
    int i;

    assert(array1->vertex_count == array2->vertex_count);
    assert(array1->vertex_count == array3->vertex_count);

    assert(array1->face_count == array2->face_count);
    assert(array1->face_count == array3->face_count);

    // should assert that face indices are the same

    if (array3->buffer[0])
    {
        glDeleteBuffers(4, array3->buffer);
        array3->buffer[0] =
        array3->buffer[1] =
        array3->buffer[2] =
        array3->buffer[3] = 0;
    }

    Box box = MIN_BOX;
    int vertex_count = array1->vertex_count;
    Vector * vertices1 = array1->vertices;
    Vector * vertices2 = array2->vertices;
    Vector * vertices3 = array3->vertices;

    for (i = 0; i != vertex_count; ++ i)
    {
        Vector vertex = vector_interpolate(vertices1[i], vertices2[i], t);
        box = box_add(box, vertex);
        vertices3[i] = vertex;
    }

    if (array1->normals)
    {
        Vector * normals1 = array1->normals;
        Vector * normals2 = array2->normals;
        Vector * normals3 = array3->normals;

        for (i = 0; i != vertex_count; ++ i)
        {
            normals3[i] = vector_normalize(vector_interpolate(normals1[i], normals2[i], t));
        }
    }

    // handle colors
}

Array * array_copy(Array const * array)
{
    int vertex_count = array->vertex_count;

    Array * copy = malloc_size(Array);
    *copy = *array;

    copy->vertices = copy_array(Vector, copy->vertices, vertex_count);

    if (array->normals)
        copy->normals = copy_array(Vector, copy->normals, vertex_count);

    if (array->tex_coords)
        copy->tex_coords = copy_array(Vector, copy->tex_coords, vertex_count);

    if (array->colors)
        copy->colors = copy_array(Color, copy->colors, vertex_count);

    if (array->faces.indices)
        copy->faces.indices = copy_array(int, copy->faces.indices, array_index_count(array));

    return copy;
}

Array * array_new(GLenum mode)
{
    Array * array = malloc_size(Array);
    clear(Array, array);
    array->mode = mode;

    return array;
}

Box array_box(Array const * array)
{
    return box_from_vertices(array->vertices, array->vertex_count);
}

void array_append_quad_indices(Array * array, int a, int b, int c, int d)
{
    error_check(array->mode != GL_QUADS, "not a quad array");

    Quad q = {a, b, c, d};

    int const face_count = array->face_count;
    Quad * quads = realloc_array(Quad, array->faces.quads, (face_count + 1));
    quads[face_count] = q;
    
    array->face_count ++;
    array->faces.quads = quads;
}

void array_append_quad(Array * array, Vector a, Vector b, Vector c, Vector d)
{
    error_check(array->mode != GL_QUADS, "not a quad array");
    error_check(array->normals != NULL, "has normals");
    error_check(array->colors != NULL, "has colors");
    error_check(array->tex_coords != NULL, "has texture coordinates");

    int const vertex_count = array->vertex_count;
    Vector * vertices = realloc_array(Vector, array->vertices, vertex_count + 4);
    vertices[vertex_count + 0] = a;
    vertices[vertex_count + 1] = b;
    vertices[vertex_count + 2] = c;
    vertices[vertex_count + 3] = d;

    if (array->faces.indices)
    {
        int const face_count = array->face_count;
        int * indices = realloc_array(int, array->faces.indices, (face_count + 1) * 4);
        indices[face_count * 4 + 0] = vertex_count + 0;
        indices[face_count * 4 + 1] = vertex_count + 1;
        indices[face_count * 4 + 2] = vertex_count + 2;
        indices[face_count * 4 + 3] = vertex_count + 3;
    
        array->face_count ++;
        array->faces.indices = indices;
    }

    array->vertex_count += 4;
    array->vertices = vertices;
}

void array_append_triangle(Array * array, Vector a, Vector b, Vector c)
{
    error_check(array->mode != GL_TRIANGLES, "not a triangle array");
    error_check(array->normals != NULL, "has normals");
    error_check(array->colors != NULL, "has colors");
    error_check(array->tex_coords != NULL, "has texture coordinates");

    int const vertex_count = array->vertex_count;
    Vector * vertices = realloc_array(Vector, array->vertices, vertex_count + 3);
    vertices[vertex_count + 0] = a;
    vertices[vertex_count + 1] = b;
    vertices[vertex_count + 2] = c;

    if (array->faces.indices)
    {
        int const face_count = array->face_count;
        int * indices = realloc_array(int, array->faces.indices, (face_count + 1) * 3);
        indices[face_count * 3 + 0] = vertex_count + 0;
        indices[face_count * 3 + 1] = vertex_count + 1;
        indices[face_count * 3 + 2] = vertex_count + 2;
    
        array->face_count ++;
        array->faces.indices = indices;
    }

    array->vertex_count += 3;
    array->vertices = vertices;
}

void array_append_line(Array * array, Vector a, Vector b)
{
    error_check(array->mode != GL_LINES, "not a line array");
    error_check(array->normals != NULL, "has normals");
    error_check(array->colors != NULL, "has colors");
    error_check(array->tex_coords != NULL, "has texture coordinates");

    int const vertex_count = array->vertex_count;
    Vector * vertices = realloc_array(Vector, array->vertices, vertex_count + 2);
    vertices[vertex_count + 0] = a;
    vertices[vertex_count + 1] = b;

    if (array->faces.indices)
    {
        int const face_count = array->face_count;
        int * indices = realloc_array(int, array->faces.indices, (face_count + 1) * 2);
        indices[face_count * 2 + 0] = vertex_count + 0;
        indices[face_count * 2 + 1] = vertex_count + 1;
    
        array->face_count ++;
        array->faces.indices = indices;
    }

    array->vertex_count += 2;
    array->vertices = vertices;
}

int array_append_point(Array * array, Vector a)
{
//    error_check(array->mode != GL_POINTS, "not a point array");
    error_check(array->normals != NULL, "has normals");
    error_check(array->colors != NULL, "has colors");
    error_check(array->tex_coords != NULL, "has texture coordinates");

    int const vertex_count = array->vertex_count;
    Vector * vertices = realloc_array(Vector, array->vertices, vertex_count + 1);
    vertices[vertex_count] = a;

    if (array->faces.indices)
    {
        int const face_count = array->face_count;
        int * indices = realloc_array(int, array->faces.indices, face_count + 1);
        indices[face_count] = vertex_count;
    
        array->face_count ++;
        array->faces.indices = indices;
    }

    array->vertex_count ++;
    array->vertices = vertices;

    return vertex_count;
}

int array_append_vertex(Array * array, Vector a)
{
//    error_check(array->mode != GL_POINTS, "not a point array");
    error_check(array->normals != NULL, "has normals");
    error_check(array->colors != NULL, "has colors");
    error_check(array->tex_coords != NULL, "has texture coordinates");

    int const vertex_count = array->vertex_count;
    Vector * vertices = realloc_array(Vector, array->vertices, vertex_count + 1);
    vertices[vertex_count] = a;

    array->vertex_count ++;
    array->vertices = vertices;

    return vertex_count;
}

Array * array_flatten_indices(Array const * source)
{
    error_check(source->faces.indices == NULL, "has no indices");

    Array * target = array_new(source->mode);

    int const * indices = source->faces.indices;
    int index_count = array_index_count(source);

    Vector const * source_vertices   = source->vertices;
    Vector const * source_normals    = source->normals;
    Vector const * source_tex_coords = source->tex_coords;
    Color  const * source_colors     = source->colors;

    Vector * target_vertices = malloc_array(Vector, index_count);
    Vector * target_normals  = source->normals ? malloc_array(Vector, index_count) : NULL;
    Vector *  target_tex_coords = source->tex_coords  ? malloc_array(Vector, index_count) : NULL;
    Color *  target_colors   = source->colors  ? malloc_array(Color,  index_count) : NULL;

    for (int i = 0; i != index_count; ++ i)
    {
        int index = indices[i];
        target_vertices[i] = source_vertices[index];

        if (target_normals)
            target_normals[i] = source_normals[index];

        if (target_tex_coords)
            target_tex_coords[i]  = source_tex_coords[index];

        if (target_colors)
            target_colors[i]  = source_colors[index];
    }
    
    target->vertices     = target_vertices;
    target->normals      = target_normals;
    target->tex_coords   = target_tex_coords;
    target->colors       = target_colors;
    target->vertex_count = index_count;

    return target;
}

void array_flip_normals(Array * array)
{
    if (! array->normals)
        return;

    int vertex_count = array->vertex_count;
    Vector * normals = array->normals;

    for (int i = 0; i != vertex_count; ++ i)
    {
        normals[i] = vector_scale(normals[i], -1);
    }
}

#ifdef FOO_BAR
void foo_bar(void)
{
    float r = 1.0;

    int edge_segment_count = 1; //0 ...
    int const edge_vertex_count = edge_segment_count + 1;

    int vertex_count = edge_vertex_count * (edge_vertex_count + 1) / 2;
    int triangle_count = SQ(edge_segment_count);

    // produce vertices
    for (int i = 0; i != edge_vertex_count; ++ i)
    {
        for (int j = 0; j != i + 1; ++ j)
        {
            Vector v = {x, y, z};
        }
    }
}
#endif

#if 0

// octagon is dual to cube
// edges are quad patches
// we can create rounded octagons and rounder cubes

// primal/dual complex has 48 edges, 12+6 = 18 quads, 8 triangles, 24 vertices

Array * array_box_rounded(Box box, float radius)
{
    Box inner_box = box;

    inner_box.min.x += radius;
    inner_box.min.y += radius;
    inner_box.min.z += radius;

    inner_box.max.x -= radius;
    inner_box.max.y -= radius;
    inner_box.max.z -= radius;

    Array * array = array_new(GL_TRIANGLES);

    int vertex_count = 0;
    Vector * vertices = NULL;

    // append 6 faces [-X, +X, -Y, +Y, -Z, +Z]

    // 12 edges [-Y-Z, +Y-Z, -Y+Z, +Y+Z | -Z-X, +Z-X, -Z+X, +Z+X | -X-Y, +X-Y, -X+Y, +X+Y]

    // 8 corners [-X-Y-Z, +X-Y-Z, -X+Y-Z, -X+Y+Z, ...]

    // dual of cube is octagon
    // corner <-> 3 edges, 3 faces      dual face
    // edge <-> 2 faces, 2 vertices
    // face <-> 4 edges, 4 vertices

    // tesselation level: 0 -> only faces
    // tesselation level: 1 or bigger ->includes edges and corners
    

    return array;
}
#endif

Array * array_to_triangles(Array const * array)
{
    error_check(array->faces.quads == NULL, "need quad indices");

    int face_count = 0;
    Face * triangles = NULL;

    int vertex_count = 0;
    Vector * vertices = NULL, * normals = NULL, * tex_coords = NULL;
    Color * colors = NULL;

    switch (array->mode)
    {
        default:
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
            return NULL;

        case GL_QUADS:
            {
                vertex_count = array->vertex_count;
                face_count   = array->face_count * 2;

                if (! face_count)
                    break;

                vertices  = copy_array(Vector, array->vertices, array->vertex_count);
                triangles = malloc_array(Face, face_count);

                if (array->normals)
                    normals = copy_array(Vector, array->normals, array->vertex_count);

                if (tex_coords)
                    tex_coords = copy_array(Vector, array->tex_coords, array->vertex_count);

                if (array->colors)
                    colors = copy_array(Color, array->colors, array->vertex_count);

                int i, j;
                for (i = 0, j = 0; i != array->face_count; i += 1, j += 2)
                {
                    Quad q = array->faces.quads[i];

                    Face t1 = {q.a, q.b, q.c};
                    Face t2 = {q.a, q.c, q.d};

                    triangles[j + 0] = t1;
                    triangles[j + 1] = t2;
                }
            }
            break;
    }

    Array * new_array = array_new(GL_TRIANGLES);
    new_array->vertex_count = vertex_count;
    new_array->face_count = face_count;
    new_array->vertices   = vertices;
    new_array->normals    = normals;
    new_array->tex_coords = tex_coords;
    new_array->colors     = colors;
    new_array->faces.triangles = triangles;

    return new_array;
}
