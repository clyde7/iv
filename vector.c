/* Copyright Claude Knaus. All rights reserved. */

#include <float.h>
#include <stdio.h>

#include "error.h"
#include "memory.h"
#include "opengl.h"
#include "vector.h"

//#define USE_FMA

Vector const
    ORIGIN = {0, 0, 0},
    VECTOR_ONE = {1, 1, 1},
    VECTOR_X = {1, 0, 0},
    VECTOR_Y = {0, 1, 0},
    VECTOR_Z = {0, 0, 1},
    MIN_VECTOR = {-FLT_MAX, -FLT_MAX, -FLT_MAX},
    MAX_VECTOR = {+FLT_MAX, +FLT_MAX, +FLT_MAX};

void vector_buffer_append(Vector_Buffer * buffer, Vector v)
{
    if (buffer->count == buffer->size)
    {
        buffer->size = (buffer->size == 0) ? 1 : buffer->size * 2;
        buffer->vectors = realloc_array(Vector, buffer->vectors, buffer->size);
    }

    buffer->vectors[buffer->count ++] = v;
}

Vector vector(float x, float y, float z)
{
    Vector v = {x, y, z};
    return v;
}

void vector_print(Vector v)
{
    printf("%.4g, %.4g, %.4g", v.x, v.y, v.z);
}

void vector_apply(Vector v)
{
    glVertex3fv((GLfloat const *) &v);
}

Vector vector_add(Vector a, Vector b)
{
    Vector v =
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };

    return v;
}

Vector vector_add3(Vector a, Vector b, Vector c)
{
    Vector v =
    {
        a.x + b.x + c.x,
        a.y + b.y + c.y,
        a.z + b.z + c.z
    };

    return v;
}

Vector vector_add4(Vector a, Vector b, Vector c, Vector d)
{
    Vector v =
    {
        a.x + b.x + c.x + d.x,
        a.y + b.y + c.y + d.y,
        a.z + b.z + c.z + d.z
    };

    return v;
}

void vector_accumulate(Vector * accumulator, Vector v, float factor)
{
#ifdef USE_FMA
    accumulator->x = fmaf(v.x, factor, accumulator->x);
    accumulator->y = fmaf(v.y, factor, accumulator->y);
    accumulator->z = fmaf(v.z, factor, accumulator->z);
#else
    accumulator->x += v.x * factor;
    accumulator->y += v.y * factor;
    accumulator->z += v.z * factor;
#endif
}

Vector vector_sub(Vector a, Vector b)
{
    Vector v =
    {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };

    return v;
}

Vector vector_negate(Vector a)
{
    Vector v =
    {
        - a.x,
        - a.y,
        - a.z
    };

    return v;
}

Vector vector_scale(Vector a, float scale)
{
    Vector v =
    {
        a.x * scale,
        a.y * scale,
        a.z * scale
    };

    return v;
}

float vector_scalar_product(Vector a, Vector b)
{
    return
        a.x * b.x +
        a.y * b.y +
        a.z * b.z;
}

Vector vector_product(Vector a, Vector b)
{
    Vector v =
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };

    return v;
}

Vector vector_mul(Vector a, Vector b)
{
    Vector v =
    {
        a.x * b.x,
        a.y * b.y,
        a.z * b.z
    };

    return v;
}

Vector vector_div(Vector a, Vector b)
{
    Vector v =
    {
        a.x / b.x,
        a.y / b.y,
        a.z / b.z
    };

    return v;
}

float vector_square(Vector v)
{
    return dot(v, v);
}

Vector vector_square_2(Vector v)
{
    v.x = sq(v.x);
    v.y = sq(v.y);
    v.z = sq(v.z);

    return v;
}

float vector_length(Vector a)
{
    return sqrt(dot(a, a));
}

float vector_distance(Vector a, Vector b)
{
    return vector_length(vector_sub(b, a));
}

float vector_distance2(Vector a, Vector b)
{
    return vector_square(vector_sub(b, a));
}

float vector_angle(Vector a, Vector b)
{
    float c = dot(a, b) / (norm(a) * norm(b));
    return acos(c);
}

float vector_angle_2(Vector a, Vector b)
{
    return atan2(vector_length(cross(a, b)), dot(a, b));
}

float vector_angle_accurate(Vector a, Vector b)
{
    Vector axb = cross(a, b);
    float t = axb.z / dot(a, b);
    return atan(t);
}

float vector_angle_signed(Vector a, Vector b)
{
    float phi_a = atan2(a.y, a.x);
    float phi_b = atan2(b.y, b.x);

    float delta_angle = phi_b - phi_a;

    if (delta_angle > M_PI)
        delta_angle -= 2 * M_PI;

    if (delta_angle < -M_PI)
        delta_angle += 2 * M_PI;

    return delta_angle;
}

Vector vector_normalize(Vector a)
{
#if 1
    float const length = vector_length(a);
    return vector_scale(a, 1.0 / length);
#else
    // FASTER?
    float length = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
    float factor = 1.0 / length;

    a.x *= factor;
    a.y *= factor;
    a.z *= factor;

    return a;
#endif
}

Vector vector_normal(Vector a, Vector b)
{
    return vector_normalize(cross(a, b));
}

Vector vector_interpolate(Vector a, Vector b, float t)
{
    float const u = 1 - t;
    Vector v =
    {
        a.x * u + b.x * t,
        a.y * u + b.y * t,
        a.z * u + b.z * t
    };

    return v;
}

Vector vector_interpolate_spherical(Vector a, Vector b, float t)
{
    float phi = vector_angle(a, b);
    float s = sin(phi);

    float factor_a = sin((1 - t) * phi) / s;
    float factor_b = sin(t * phi) / s;

    return vector_add(vector_scale(a, factor_a), vector_scale(b, factor_b));
}

Vector vector_interpolate_bilinear(Vector const vectors[2][2], float u, float v)
{
    Vector v0 = vector_interpolate(vectors[0][0], vectors[0][1], u);
    Vector v1 = vector_interpolate(vectors[1][0], vectors[1][1], u);

    return vector_interpolate(v0, v1, v);
}

Vector vector_add_scaled(Vector a, Vector b, float f)
{
    Vector v =
    {
#ifdef USE_FMA
        fmaf(b.x, f, a.x),
        fmaf(b.y, f, a.y),
        fmaf(b.z, f, a.z)
#else
        a.x + b.x * f,
        a.y + b.y * f,
        a.z + b.z * f
#endif
    };

    return v;
}

Vector vector_min(Vector a, Vector b)
{
    Vector v =
    {
        fmin(a.x, b.x),
        fmin(a.y, b.y),
        fmin(a.z, b.z)
    };

    return v;
}

Vector vector_max(Vector a, Vector b)
{
    Vector v =
    {
        fmax(a.x, b.x),
        fmax(a.y, b.y),
        fmax(a.z, b.z)
    };

    return v;
}

Vector vector_random_position(void)
{
    Vector v =
    {
        random_range(-1, 1),
        random_range(-1, 1),
        random_range(-1, 1)
    };

    return v;
}

Vector vector_random_direction(void)
{
    float x, y, z;

    do
    {
        x = random_range(-1, 1);
        y = random_range(-1, 1);
        z = random_range(-1, 1);
    }
    while (x*x + y*y + z*z > 1);

    return vector_normalize(vector(x, y, z));
}

Vector vector_random_direction_2(Seed * seed)
{
    float x, y, z;

    do
    {
        x = random_range_seed(seed, -1, 1);
        y = random_range_seed(seed, -1, 1);
        z = random_range_seed(seed, -1, 1);
    }
    while (x*x + y*y + z*z > 1);

    return vector_normalize(vector(x, y, z));
}

Vector vector_random_spot_direction(Seed * seed, Vector direction, float cos_width)
{
    Vector v;

    do
    {
        v = vector_random_direction_2(seed);
    }
    while (dot(direction, v) < cos_width);

    return v;
}

Vector vector_random_hemi_direction(Seed * seed, Vector n)
{
    float x, y, z;

    do
    {
        x = random_range_seed(seed, -1, 1);
        y = random_range_seed(seed, -1, 1);
        z = random_range_seed(seed, -1, 1);
    }
    while (x*n.x + y*n.y + z*n.z <= 0.001 && x*x + y*y + z*z > 1);

    return vector_normalize(vector(x, y, z));
}

int vector_min_index(Vector v)
{
    int index = -1;
    float value = FLT_MAX;

    if (fabs(v.x) < value)
    {
        value = fabs(v.x);
        index = 0;
    }

    if (fabs(v.y) < value)
    {
        value = fabs(v.y);
        index = 1;
    }

    if (fabs(v.z) < value)
    {
        value = fabs(v.z);
        index = 2;
    }

    return index;
}

Vector vector_dimension(int dimension)
{
    switch (dimension)
    {
        default:
        case 0: return VECTOR_X;
        case 1: return VECTOR_Y;
        case 2: return VECTOR_Z;
    }
}

Vector vector_perpendicular(Vector v)
{
    int index = vector_min_index(v);
    error_check(index == -1, "cannot make zero vector perpendicular");

    Vector u = vector_dimension(index);

    return vector_normalize(cross(v, u));
}

void vector_orthogonals(Vector z, Vector * x, Vector * y)
{
    *x = vector_perpendicular(z);
    *y = cross(z, *x);
}

Vector vector_transform(Vector ex, Vector ey, Vector ez, Vector r)
{
    Vector v =
    {
        ex.x * r.x + ey.x * r.y + ez.x * r.z,
        ex.y * r.x + ey.y * r.y + ez.y * r.z,
        ex.z * r.x + ey.z * r.y + ez.z * r.z
    };

    return v;
}

Vector vector_random_disk(Seed * seed)
{
    float u = random_seed(seed) * 2 - 1;
    float v = random_seed(seed) * 2 - 1;

    float r, phi;

    if (u >= -v)
    {
        if (u > v) { r = u; if (v > 0) phi = v/r; else phi = 8.0 + v/r; }
        else       { r = v; phi = 2.0 - u/r; }
    }
    else
    {
        if (u <= v) { r = -u; phi = 4.0 - v/r; }
        else        { r = -v; phi = 6.0 + u/r; }
    }

    phi *= M_PI / 4.0;

    Vector w = {r * cos(phi), r * sin(phi), 0};
    return w;
}

Vector vector_random_hemi_direction_cos_pbrt(Seed * seed, Vector n)
{
    Vector v = vector_random_disk(seed);
#if 0
    // XXX hack
    v = vector_scale(v, 0.999);
#endif
    Vector w = {v.x, v.y, sqrt(fmax(0, 1 - v.x * v.x - v.y * v.y))};

    Vector x = vector_perpendicular(n);
    Vector y = cross(n, x);
    Vector z = n;
    
    //return vector_transform(x, y, z, w);
    return vector_normalize(vector_transform(x, y, z, w));
}

Vector vector_random_hemi_direction_cos(Seed * seed, Vector n)
{
    float u = random_seed(seed);
    float v1 = random_seed(seed);

    float r = sqrt(u);
    float phi = 2 * M_PI * v1;

    Vector v = vector_polar(phi, r);
    Vector w = {v.x, v.y, sqrt(fmax(0, 1 - v.x * v.x - v.y * v.y))};

    Vector x = vector_perpendicular(n);
    Vector y = cross(n, x);
    Vector z = n;
    
    //return vector_transform(x, y, z, w);
    return vector_normalize(vector_transform(x, y, z, w));
}

Vector vector_random_hemi_direction_2(Seed * seed, Vector n)
{
    float u = 0; while (u < 0.001) u = random_seed(seed);
    float v = random_seed(seed);

    float r = sqrt(1 - u * u);
    float phi = 2 * M_PI * v;

    Vector w = vector_polar(phi, r);
    w.z = u;

    Vector x = vector_perpendicular(n);
    Vector y = cross(n, x);
    Vector z = n;
    
    //return vector_transform(x, y, z, w);
    return vector_normalize(vector_transform(x, y, z, w));
}

Vector vector_polar(float phi, float radius)
{
    Vector v =
    {
        radius * cos(phi),
        radius * sin(phi),
        0
    };

    return v;
}

Vector vector_spheric(float phi, float theta, float radius)
{
    Vector v =
    {
        radius * cos(phi) * cos(theta),
        radius * sin(phi) * cos(theta),
        radius * sin(theta)
    };

    return v;
}

// TODO remove normalize, use vector_add_scaled
Vector vector_reflect(Vector normal, Vector incoming)
{
    float factor = - 2 * dot(normal, incoming);
    return vector_normalize(vector_add(vector_scale(normal, factor), incoming));
}

Vector vector_reflect_classic(Vector N, Vector V)
{
   return vector_sub(vector_scale(N, 2 * dot(N, V)), V);
}


Vector vector_halfway(Vector a, Vector b)
{
    return vector_normalize(vector_add(a, b));
}

Vector vector_direction(Vector a, Vector b)
{
    return vector_normalize(vector_sub(b, a));
}

int vector_side(Vector a, Vector b, Vector p)
{
    Vector ab = vector_sub(b, a);
    Vector n = {ab.y, -ab.x, 0};
    n = vector_normalize(n);

    Vector ap = vector_sub(p, a);

    return signum(dot(n, ap));
}

Vector vector_clamp(Vector v, float max_length)
{
    float length = vector_length(v);

    return length > max_length
        ? vector_scale(v, max_length / length)
        : v;
}

Vector vector_homogenize(Vector v)
{
    if (v.z == 0)
        return v;

    v.x /= v.z;
    v.y /= v.z;
    v.z = 1;

    return v;
}

float vector_min_component(Vector v)
{
    return fmin(v.x, fmin(v.y, v.z));
}

float vector_max_component(Vector v)
{
    return fmax(v.x, fmax(v.y, v.z));
}

Vector vector_random_area(Seed * seed, Vector const corners[2][2])
{
    float u = random_seed(seed);
    float v = random_seed(seed);

    return vector_interpolate_bilinear(corners, u, v);
}

int vector_is_nan(Vector v)
{
#ifdef WINDOWS
    return 0; // XXX
#else
    return
        isnan(v.x) ||
        isnan(v.y) ||
        isnan(v.z);
#endif
}

Vector vector_floor(Vector v)
{
    v.x = floor(v.x);
    v.y = floor(v.y);
    v.z = floor(v.z);

    return v;
}

Vector vector_ceil(Vector v)
{
    v.x = ceil(v.x);
    v.y = ceil(v.y);
    v.z = ceil(v.z);

    return v;
}

Vector triangle_normal(Vector a, Vector b, Vector c)
{
    Vector ab = SUB(b, a);
    Vector ac = SUB(c, a);

    return vector_normalize(cross(ab, ac));
}

Vector vector_project(Vector n, Vector v)
{
    return vector_add_scaled(v, n, -dot(n, v));
}

Vector vector_update_sample_mean(Vector mean_n, Vector x_n1, int n)
{
     return vector_scale(vector_add(vector_scale(mean_n, n), x_n1), 1.0 / (n + 1));
}

Vector vector_update_sample_variance(Vector var_n, Vector mean_n1, Vector x_n1, int n)
{
    if (n == 0)
        return ORIGIN;

     Vector a = vector_scale(var_n, (float) n / (n + 1));
     Vector b = vector_scale(vector_square_2(vector_sub(x_n1, mean_n1)), 1.0 / n);
     return vector_add(a, b);
}

