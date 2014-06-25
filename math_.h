#ifndef MATH_H
#define MATH_H

#include <math.h>
//#include "complex_.h"

#ifdef CYGWIN
#include <ieeefp.h>
#endif

#ifdef WINDOWS
#include <float.h>
#define isnan _isnan
#define log2(x) (log(x)/log(2.0f))
#endif

//#ifdef Complex
//#undef Complex
//#endif

#ifndef M_PI
#define M_PI (3.14159265359)
#endif

#ifndef M_E
#define M_E (2.71828182846)
#endif

#ifndef M_SQRT2
#define M_SQRT2 (1.4142135623730)
#endif

#ifndef M_SQRT5
#define M_SQRT5 (2.2360679774997)
#endif

#ifndef M_PHI
#define M_PHI (1.61803398875)
#endif

#ifndef NAN
extern float NAN;
#endif

#ifndef INFINITY
extern float INFINITY;
#endif

#define SQ(x) ((x)*(x))
#ifndef CUDA
#define fmax(a, b) (((a) > (b)) ? (a) : (b))
#define fmin(a, b) (((a) < (b)) ? (a) : (b))
#endif
#define imax(a, b) (((a) > (b)) ? (a) : (b))
#define imin(a, b) (((a) < (b)) ? (a) : (b))

#define signum(x) (((x) < 0) ? -1 : ((x) > 0) ? 1 : 0)

#define linear_ramp(x) (x)
#define square_ramp(x) ((x) * (x))
#define square_root_ramp(x) (sqrt(x))
#define cos_ramp(x) ((1 - cos((x) * M_PI)) / 2)

#define gaussian_2d(s2, r2) ((1.0 / (2.0 * M_PI * (s2))) * exp(-(r2)/(2.0 * (s2))))

#define deg2rad(x) ((x) * M_PI / 180)
#define rad2deg(x) ((x) * 180.0 / M_PI)

#define round_(x) ((int) ((x) + 0.5))

#define LERP(a, b, t) (a) * (1 - (t)) + (b) * (t)

#define ROTATE_SHORT_LEFT(a) (((a) << 1) | ((a) >> 15))
#define ROTATE_SHORT_RIGHT(a) (((a) << 15) | ((a) >> 1))

#define CLAMP(x) (x) < 0 ? -1 : (x) > 1 ? 1 : (x)

#define Beta(x, y) (tgamma(x) * tgamma(y) / tgamma((x) + (y)))

typedef unsigned long long Seed;
//typedef struct { Complex a, b, c, d; } Moebius_Transform;

float sq(float);
float gauss_1d(float m, float s, float x);
float gauss_2d(float mx, float my, float sx, float sy, float x, float y);
int   power_of_two(unsigned);
unsigned round_to_power_of_two(unsigned);
float discretize(float value, float max, int modulo);
float clamp(float);
float clamp_to(float, float, float);
int   iclamp_to(int, int, int);
void  solve_quadratic(float a, float b, float c, float t[2]);
int   solve_quadratic_bernstein(float a, float b, float c, float t[2]);
int   solve_quadratic_2(float a, float b, float c, float t[2]);
int   solve_quartic(float a, float b, float c, float d, float t[4]);
int   solve_quartic_2(float a, float b, float c, float d, float t[4]);
float normalize(float a, float b, float t);
float interpolate(float a, float b, float t);
float interpolate_bilinear(float const values[2][2], float u, float v);
float randn(void);
float random2(void);
float random3(void);
float random_seed(Seed *);
float random_range(float min_value, float max_value);
float random_range_seed(Seed *, float min_value, float max_value);
float map_range(float value, float source_min, float source_max, float target_min, float target_max);
float zone_plate(float r, float k);
//Complex moebius_transform(Complex z, Moebius_Transform const *);
float haar(float);
float gamma_correct(float value, float gamma);
int modulo(int x, int m);
void float_to_frac(float, int * numerator, int * denominator);

float polynom_evaluate(float const coefficients[], int n, float x);

void math_initialize(void);
void math_test(void);

#endif
