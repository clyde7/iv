#ifndef BOX_H
#define BOX_H

#include "vector.h"
#include "matrix.h"

typedef struct {Vector min, max;} Box;

extern Box const EMPTY_BOX, UNIT_BOX, ORIGIN_BOX, MIN_BOX, MAX_BOX, MONOLITH;

int    box_parse(void const * data, char const string[], void * boxes_);
void   box_print(Box);
Box    box_fix(Box);
Box    box_union(Box, Box);
Box    box_add(Box, Vector);
Box    box_translate(Box, Vector);
Box    box_scale(Box, float);
Box    box_expand(Box, float);
Box    box_transform(Box, Matrix);
int    box_empty(Box);
int    box_overlap(Box, Box);
int    box_inside(Box, Vector);
int    box_max_extent(Box);
float  box_volume(Box);
Vector box_size(Box);
Vector box_ratio(Box);
Vector box_center(Box);
void   box_bounding_sphere(Box, Vector * center, Vector * corner);
Box    box_from_bounding_sphere(Vector center, Vector corner);
Box    box_from_vertices(Vector const vertices[], int count);
void   box_to_vertices(Box, Vector vertices[8]);
float  box_surface_area(Box);
float  box_area(Box);
Box    box_interpolate(Box, Box, float);
Vector box_interpolate_point(Box, Vector);

Vector vector_random(Box);

#endif
