#ifndef VECTOR_H
#define VECTOR_H

#include "math_.h"

typedef struct {float x, y, z;} Vector;
typedef struct {Vector * vectors; int size, count;} Vector_Buffer;

extern Vector const VECTOR_X, VECTOR_Y, VECTOR_Z, ORIGIN, MIN_VECTOR, MAX_VECTOR;

#define dot(a, b)    vector_scalar_product((a), (b))
#define cross(a, b)  vector_product((a), (b))
#define cross2(a, b) ((a).x * (b).y - (a).y * (b).x)
#define norm(a)      vector_length((a))
#define neg(a)       vector_negate((a))
#define scale(a, f)  vector_scale((a), (f))

#define SUB(a, b)    {a.x - b.x, a.y - b.y, a.z - b.z}
#define CROSS(a, b)  {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}
#define DOT(a, b)    ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)

void   vector_buffer_append(Vector_Buffer *, Vector);

Vector vector(float x, float y, float z);
void   vector_print(Vector);
void   vector_apply(Vector);
Vector vector_add(Vector, Vector);
Vector vector_add3(Vector, Vector, Vector);
Vector vector_add4(Vector, Vector, Vector, Vector);
void   vector_accumulate(Vector *, Vector, float);
Vector vector_sub(Vector, Vector);
Vector vector_negate(Vector);
Vector vector_scale(Vector, float);
Vector vector_product(Vector, Vector);
Vector vector_mul(Vector, Vector);
Vector vector_div(Vector, Vector);
float  vector_scalar_product(Vector, Vector);
float  vector_angle(Vector, Vector);
float  vector_angle_2(Vector, Vector);
float  vector_angle_accurate(Vector, Vector);
float  vector_angle_signed(Vector, Vector);
float  vector_length(Vector);
float  vector_distance(Vector, Vector);
float  vector_distance2(Vector, Vector);
float  vector_square(Vector);
Vector vector_square_2(Vector);
Vector vector_normalize(Vector);
Vector vector_normal(Vector, Vector);
Vector vector_direction(Vector, Vector);
Vector vector_interpolate(Vector, Vector, float);
Vector vector_interpolate_spherical(Vector, Vector, float);
Vector vector_interpolate_bilinear(Vector const vectors[2][2], float u, float v);
Vector vector_add_scaled(Vector, Vector, float);
Vector vector_min(Vector, Vector);
Vector vector_max(Vector, Vector);
Vector vector_random_area(Seed *, Vector const corners[2][2]);
Vector vector_random_position(void);
Vector vector_random_direction(void);
Vector vector_random_direction_2(Seed *);
Vector vector_random_spot_direction(Seed * seed, Vector direction, float cos_width);
Vector vector_random_hemi_direction(Seed *, Vector normal);
Vector vector_random_hemi_direction_2(Seed *, Vector normal);
Vector vector_random_disk(Seed *);
Vector vector_random_hemi_direction_cos(Seed *, Vector normal);
Vector vector_random_hemi_direction_cos_pbrt(Seed *, Vector normal);
Vector vector_random_hemi_direction_cos_3(Seed *, Vector normal);
Vector vector_polar(float phi, float radius);
Vector vector_spheric(float phi, float delta, float radius);
Vector vector_reflect(Vector normal, Vector);
Vector vector_reflect_classic(Vector normal, Vector);
Vector vector_halfway(Vector, Vector);
int    vector_side(Vector a, Vector b, Vector p);
Vector vector_clamp(Vector, float length);
Vector vector_rotate(Vector, Vector, float);
Vector vector_homogenize(Vector);
float  vector_min_component(Vector);
float  vector_max_component(Vector);
int    vector_min_index(Vector);
Vector vector_perpendicular(Vector);
void   vector_orthogonals(Vector, Vector *, Vector *);
Vector vector_transform(Vector x, Vector y, Vector z, Vector v);
int    vector_is_nan(Vector);
Vector vector_floor(Vector);
Vector vector_ceil(Vector);
Vector vector_project(Vector n, Vector v);
Vector vector_update_sample_mean(Vector mean_n, Vector x_n1, int n);
Vector vector_update_sample_variance(Vector var_n, Vector mean_n1, Vector x_n1, int n);

Vector triangle_normal(Vector, Vector, Vector);


#endif
