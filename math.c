#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>

#include "math_.h"
#include "utils.h"

#ifndef NAN
float NAN = sqrt(-1.0f);
#endif

#if 0
#ifndef INFINITY
float INFINITY = 1.0/0.0;
#endif
#endif

float sqrt_float_max;

void math_initialize(void)
{
    static int initialized;
    if (initialized)
        return;

    sqrt_float_max = sqrt(FLT_MAX);

    initialized = 1;
}

float sq(float f) {return f*f;}

float binomial_coefficient(float n, float k)
{
    return tgamma(n + 1) / (tgamma(k + 1) * tgamma(n - k + 1));
}

float gauss_1d(float m, float s, float x)
{
    float const dx = (x - m) / s;

    return exp(- dx * dx / 2);
}

float gauss_2d(float mx, float my, float sx, float sy, float x, float y)
{
    float const dx = (x - mx) / sx;
    float const dy = (y - my) / sy;

    return exp(-(dx * dx + dy * dy) / 2);
}

int power_of_two(unsigned value)
{
    return (value & (value - 1)) == 0;
}

unsigned round_to_power_of_two(unsigned value)
{
    unsigned n;

    for (n = 1; n < value; n <<= 1);
     
    return n;
}

float discretize(float value, float max, int modulo)
{
    int t = (int) (value * modulo / max);
    return t * max / modulo;
}

// #define clamp_to(x, x_min, x_max) ((x) < (x_min) ? (x_min) : (x) > (x_max) ? (x_max) : (x))
// #define clamp(x) clamp_to(x, 0, 1)

float clamp(float value)
{
    if (value < 0) return 0;
    if (value > 1) return 1;

    return value;
}

float clamp_to(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;

    return value;
}

int iclamp_to(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;

    return value;
}

int solve_quadratic_2(float a, float b, float c, float t[2])
{
    if (a == 0)
    {
        t[0] = -c/b;
        return 1;
    }

    float value = b*b - 4*a*c;
    if (value < 0)
        return 0;

    float det = sqrt(value);

    t[0] = (-b + det) / (2*a);
    t[1] = (-b - det) / (2*a);
    return 2;
}

void solve_quadratic(float a, float b, float c, float t[2])
{
    float det = sqrt(b*b - 4*a*c);
    float q = -0.5 * ((b < 0) ? (b - det) : (b + det));

    t[0] = q / a;
    t[1] = c / q;
}

int solve_quadratic_bernstein(float a, float b, float c, float t[2])
{
    if (b*b < a*c)
        return 0;

    float divisor = a - 2 * b + c;
    if (divisor == 0)
    {
        t[0] = a / (a - c);
        return 1;
    }

    float center = a - b;
    float det = sqrt(b*b - a*c);

    t[0] = (center + det) / divisor;
    t[1] = (center - det) / divisor;
    return 2;
}

static float const inv2 = 1.0 / 2.0;
static float const inv3 = 1.0 / 3.0;
static float const inv4 = 1.0 / 4.0;

// x^2 + b*x + c = 0 
static int quadratic(float b, float c, float rts[2], float dis)
{
   if (dis < 0)
   {
       rts[0] = rts[1] = 0;
       return 0;
   }

   float rtdis = sqrt(dis);

   rts[0] = (b > 0.0)
       ? (-b - rtdis) * inv2
       : (-b + rtdis) * inv2;

   rts[1] = (rts[0] == 0.0)
       ? -b
       : c / rts[0];

   return 2;
}

int solve_cubic(float b, float c, float d, float x[3]);

int solve_quartic_2(float p, float q, float r, float s, float t[4])
{
    // x^4 + p*x^3 + q*x^2 + r*x + s = 0 

    float a = q - p*p*3/8;
    float b = p*p*p/8 - p*q/2 + r;
    float c = p*p*p*p*(-3.0/256.0) + p*p*q/16 - p*r/4 + s;

    if (b == 0.0)
    {
        float z[2];
        int count = solve_quadratic_2(1, a, c, z);
        if (count == 0)
            return 0;

        assert(count == 2);

        int solutions = 0;
        if (z[0] >= 0)
        {
           float x1 = sqrt(z[0]);
           float x2 = -x1;
           t[0] = x1 - p/4;
           t[1] = x2 - p/4;
           solutions += 2;
        }

        if (z[1] >= 0)
        {
            float x1 = sqrt(z[1]);
            float x2 = -x1;
            t[solutions + 0] = x1 - p/4;
            t[solutions + 1] = x2 - p/4;
            solutions += 2;
        }

        return solutions;
    }

    float t3[4];
    solve_cubic(2*a, a*a - 4*c, - b*b, t3);
    float z = t3[0];

    float alpha = sqrt(z);
    float rho   = -b/alpha;
    float beta  = (a + z + rho) / 2;
    float delta = (a + z - rho) / 2;

    float t1[2] = {-1, -1}, t2[2] = {-1, -1};
    int n1 = solve_quadratic_2(1,  alpha, beta,  t1);
    int n2 = solve_quadratic_2(1, -alpha, delta, t2);

    t[0] = t1[0] - p/4;
    t[1] = t1[1] - p/4;
    t[n1 + 0] = t2[0] - p/4;
    t[n1 + 1] = t2[1] - p/4;

    return n1 + n2;
}

// x^4 + a*x^3 + b*x^2 + c*x + d = 0 
int solve_quartic(float a, float b, float c, float d, float rts[4])
{
   float a2 = a * a;
   float p = -b ;
   float q = a*c - 4*d ;
   float r = -a2*d - c*c + 4*b*d ;

   float t[3];
   solve_cubic(p, q, r, t);
   float y = t[0];

   float e2 = inv4*a2 - b + y;
   float f2 = inv4*y*y - d;

   if (e2 < 0.0 || f2 < 0.0) return 0;

   float e = sqrt(e2);
   float f = sqrt(f2);
   float g = inv2*a - e;
   float h = inv2*y - f;
   float gg = inv2*a + e;
   float hh = inv2*y + f;

   float v1[2], v2[2];
   int n1 = quadratic(gg, hh, v1, gg*gg - 4.0*hh);
   int n2 = quadratic( g,  h, v2,  g*g  - 4.0*h);

   rts[0] = v1[0];
   rts[1] = v1[1];
   rts[n1 + 0] = v2[0];
   rts[n1 + 1] = v2[1];

   return n1 + n2;
}

float normalize(float a, float b, float t)
{
    return (t - a) / (b - a);
}

float interpolate(float a, float b, float t)
{
    //return fmaf(t, b - a, a);
    return (1 - t) * a + t * b;
}

float interpolate_bilinear(float const values[2][2], float u, float v)
{
    float x0 = interpolate(values[0][0], values[0][1], u);
    float x1 = interpolate(values[1][0], values[1][1], u);

    return interpolate(x0, x1, v);
}

int noise(int x)
{
    x = (x << 13) ^ x;

    return 1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;
}

float randn(void)
{
    float x = random2();
    float y = random2();

    return sqrt(-2 * log(x)) * cos(2 * M_PI * y);
}

float random2(void)
{
    return (float) rand() / RAND_MAX;
}

// source: Realistic Ray Tracing, Peter Shirley
float random3(void)
{
    static unsigned long long seed = 7564231ULL;
    static unsigned long long mult = 62089911ULL;
    static unsigned long long llong_max = 4294967295ULL;
    static float float_max = 4294967295.0f;

    seed *= mult;
    return (float) (seed % llong_max) / float_max;
}

float random_seed(unsigned long long * seed)
{
//    assert(* seed != 0);
#if 0
    if (1)
        return random2();
#endif
    static unsigned long long const mult = 62089911ULL;
    static unsigned long long const llong_max = 4294967295ULL;
    static float const float_max = 4294967295.0f;

    * seed *= mult;
    return (float) (* seed % llong_max) / float_max;
}

float random_range(float min_value, float max_value)
{
    return interpolate(min_value, max_value, random2());
}

float random_range_seed(unsigned long long * seed, float min_value, float max_value)
{
    return interpolate(min_value, max_value, random_seed(seed));
}

float map_range(float value, float source_min, float source_max, float target_min, float target_max)
{
    return target_min + (target_max - target_min) * (value - source_min) / (source_max - source_min);
}

float zone_plate(float r, float k)
{
    return (1 + cos(k * r * r)) / 2;
}

#if 0
Complex moebius_transform(Complex z, Moebius_Transform const * t)
{
    Complex az = complex_mul(t->a, z);
    Complex cz = complex_mul(t->c, z);

    Complex numerator   = complex_add(az, t->b);
    Complex denominator = complex_add(cz, t->d);

    return complex_div(numerator, denominator);
}
#endif

float haar(float t)
{
    return
        (0 <= t && t < 0.5) ? +1 :
        (0.5 <= t && t < 1) ? -1 :
        0;
}

float gamma_correct(float value, float gamma)
{   
    return pow(value, 1.0 / gamma);
}

float polynom_evaluate(float const coefficients[], int n, float x)
{
    float accumulator = 0;
    float p = 1;

    for (int i = 0; i != n; ++ i)
    {
#ifdef USE_FMA
        accumulator = fma(coefficients[i], p, accumulator);
#else
        accumulator += coefficients[i] * p;
#endif
        p *= x;
    }

    return accumulator;
}

static float cubic_root(float x)
{
    return x < 0
        ? -exp(log(-x)/3)
        :  exp(log(x)/3);
}

int solve_cubic(float b, float c, float d, float x[3])
{
    float q = (3*c - b*b) / 9;
    float r = (9*b*c  - 27*d - 2*b*b*b) / 54;

    float delta = q*q*q + r*r;

    if (delta >= 0)
    {
        float R = sqrt(delta);
        float s = cubic_root(r + R);
        float t = cubic_root(r - R);
        x[0] = s + t - b/3;

        return 1;
    }

    float rho = sqrt(-q*q*q);
    float theta = acos(r / rho);
    float radius = pow(rho, inv3);
    float sr = radius * cos(theta / 3);
    float si = radius * sin(theta / 3);

    x[0] = 2 * sr - b/3;
    x[1] = -sr - b/3 - sqrt(3.0) * si;
    x[2] = -sr - b/3 + sqrt(3.0) * si;

    assert(! isnan(x[0]));
    assert(! isnan(x[1]));
    assert(! isnan(x[2]));

    if (x[1] < x[0]) swap(float, x[1], x[0]);
    if (x[2] < x[1]) swap(float, x[2], x[1]);
    if (x[1] < x[0]) swap(float, x[1], x[0]);

    return 3;
}

int modulo(int x, int m)
{
    int r = x % m;
    return r < 0 ? r + m : r;
}

void float_to_frac(float x, int * numerator, int * denominator)
{
    int a = 1, b = 1;

    while (1)
    {
        float f = (float) a / b;

        if (f < x)
            ++ a;
        else if (f > x)
        {
            ++ b;
            a = floor(x * b);
        }
        else
            break;
    }

    * numerator   = a;
    * denominator = b;
}

void float_to_frac_test(void)
{
    int a, b;
    float x;

    x = (float) 4/3;  float_to_frac(x, &a, &b); printf("%f = %d / %d\n", x, a, b);
    x = (float) 16/9; float_to_frac(x, &a, &b); printf("%f = %d / %d\n", x, a, b);
}

// tests

static void cubic_test(void)
{
    // (x - 1)(x - 2)(x - 3) = (x^2 - 3x + 2)(x - 3) = (x^3 - 6x^2 + 11x - 6)
    // solve_cubic(-6, 11, -6)

    float x[3];

    int count = solve_cubic(-6, 11, -6, x);
    printf("%d cubic solutions\n", count);
    for (int i = 0; i != count; ++ i)
    {
        printf("cubic solution %d: %f\n", i, x[i]);
    }

    // (x - 3)(x^2 + 1) = (x^3 - 3x^2 + x - 3) 
    count = solve_cubic(-3, 1, -3, x);
    printf("%d cubic solutions\n", count);
    for (int i = 0; i != count; ++ i)
    {
        printf("cubic solution %d: %f\n", i, x[i]);
    }

    // TOOD test : nan? -5.000000, 100.000000, -15625.000000
    count = solve_cubic(-5, 100, -15625, x);
    printf("%d cubic solutions\n", count);
    for (int i = 0; i != count; ++ i)
    {
        printf("cubic solution %d: %f\n", i, x[i]);
    }
}

static void quartic_test(void)
{
    // (x - 1)(x - 2)(x - 3)(x - 4) = (x^2 - 3x + 2)(x^2 - 7x + 12)
    // = (x^4 - 10x^3 + 35x^2 - 50x + 24) = 0
    float t[4];
    int count = solve_quartic_2(-10, 35, -50, 24, t);
    //int count = solve_quartic(-10, 35, -50, 24, t);

    printf("%d solutions\n", count);
    for (int i = 0; i != count; ++ i)
    {
        printf("solution %d: %f\n", i, t[i]);
    }

    // XXX fails
}

void math_test(void)
{
    float_to_frac_test();
    cubic_test();
    quartic_test();
}

