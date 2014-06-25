#include <float.h>
#include <stdio.h>

#include "color.h"
#include "math_.h"
#include "matrix.h"
#include "opengl.h"

static float const GAMMA = 2.2f;
float const U_AMPLITUDE = 0.436f;
float const V_AMPLITUDE = 0.615f;

Color const BLACK   = {0, 0, 0};
Color const RED     = {1, 0, 0};
Color const GREEN   = {0, 1, 0};
Color const YELLOW  = {1, 1, 0};
Color const BLUE    = {0, 0, 1};
Color const MAGENTA = {1, 0, 1};
Color const CYAN    = {0, 1, 1};
Color const WHITE   = {1, 1, 1};
Color const GRAY    = {0.5, 0.5, 0.5};
Color const ORANGE  = {1, 0.5, 0};
Color const VIOLET  = {0.58, 0, 0.828};
Color const MIN_COLOR = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
Color const MAX_COLOR = {+FLT_MAX, +FLT_MAX, +FLT_MAX};

#define F (1.0 / 0.17697)
static Matrix const RGB_TO_XYZ =
{
    {
        {0.49 * F, 0.17697 * F, 0    * F, 0},
        {0.31 * F, 0.81240 * F, 0.01 * F, 0},
        {0.20 * F, 0.01063 * F, 0.99 * F, 0},
        {0, 0, 0, 1}
    }
};
#undef F

static Matrix const RGB_TO_YUV =
{
    {
        {0.299, -0.14713, 0.615, 0},
        {0.587, -0.28886, -0.51499, 0},
        {0.114, 0.436, -0.10001, 0},
        {0, 0, 0, 1}
    }
};

static Matrix const YUV_TO_RGB =
{
    {
        {1, 1, 1, 0},
        {0, -0.39465, 2.03211, 0},
        {1.13983, -0.58060, 0, 0},
        {0, 0, 0, 1}
    }
};

static Matrix const XYZ_TO_LMS =
{
    {
        {0.38971, -0.22981, 0, 0},
        {0.68898, 1.18340, 0, 0},
        {-0.07868, 0.04641, 1, 0},
        {0, 0, 0, 1}
    }
};

Color4 color4(Color color, float alpha)
{
    Color4 c = {color, alpha};
    return c;
}

void color_print(Color color)
{
    printf("{%.8f, %.8f, %.8f}", color.r, color.g, color.b);
}

void color_print_pattern(Color color, char const pattern[])
{
    printf(pattern, color.r, color.g, color.b);
}

void color_print_name(Color color)
{
    color_print(color); 
    printf(" (%s)", color_to_svg_name(color));
}

void color_apply(Color color)
{
    glColor3fv((GLfloat const *) &color);
}

void color4_apply(Color4 color)
{
    glColor4fv((GLfloat const *) &color);
}

void color_apply_alpha(Color color, float alpha)
{
    glColor4f(color.r, color.g, color.b, alpha);
}

void color_table(Color_Map get, unsigned char table[256][3])
{
    unsigned i;

    for (i = 0; i != 256; ++ i)
    {
        Color const color = get(i);
        table[i][0] = (unsigned char) (color.r * 255);
        table[i][1] = (unsigned char) (color.g * 255);
        table[i][2] = (unsigned char) (color.b * 255);
    }
}

void color_table_transposed(Color_Map get, unsigned char table[3][256])
{
    unsigned i;

    for (i = 0; i != 256; ++ i)
    {
        Color const color = get(i);
        table[0][i] = (unsigned char) (color.r * 255);
        table[1][i] = (unsigned char) (color.g * 255);
        table[2][i] = (unsigned char) (color.b * 255);
    }
}

Color color_from_rgb(float red, float green, float blue)
{
    Color c = {red, green, blue};
    return c;
}

void color_to_cmyk(Color color, float cmyk[4])
{
    float c = 1 - color.r;
    float m = 1 - color.g;
    float y = 1 - color.b;
    float k = fmin(c, fmin(m, y));

    cmyk[0] = c - k;
    cmyk[1] = m - k;
    cmyk[2] = y - k;
    cmyk[3] = k;
}

Color color_from_cmyk(float const cmyk[4])
{
    float k = cmyk[3];
    Color c =
    {
        1 - (cmyk[0] + k),
        1 - (cmyk[1] + k),
        1 - (cmyk[2] + k)
    };

    return c;
}

void color_to_hsv(Color c, float * hue, float * saturation, float * value)
{
    float max_value = fmax(fmax(c.r, c.g), c.b);
    float min_value = fmin(fmin(c.r, c.g), c.b);

    * value = max_value;

    if (max_value == 0)
    {
        * saturation = 0;
        * hue = -1;
        return;
    }

    float range = max_value - min_value;

    * saturation = range / max_value;

    if (c.r == max_value)
        * hue = (c.g - c.b) / range;
    else if (c.g == max_value)
        * hue = 2 + (c.b - c.r) / range;
    else
        * hue = 4 + (c.r - c.g) / range;

    * hue *= 60;
    if (* hue < 0)
        * hue += 360;
}

Color color_from_hsv(float hue, float saturation, float value)
{
    hue = fmod(hue, 360);

    int index = ((int) floor(hue / 60));
    float f = hue / 60 - floor(hue / 60);

    float p = value * (1 - saturation);
    float q = value * (1 - f * saturation);
    float t = value * (1 - (1 - f) * saturation);

    switch (index)
    {
        default: return color_from_rgb(1, 0, 1); /* error */
        case 0: return color_from_rgb(value, t, p);
        case 1: return color_from_rgb(q, value, p);
        case 2: return color_from_rgb(p, value, t);
        case 3: return color_from_rgb(p, q, value);
        case 4: return color_from_rgb(t, p, value);
        case 5: return color_from_rgb(value, p, q);
    }
}

void color_to_orgb(Color color, float l_cyb_crg[3])
{
    static Matrix const m =
    {
        {
            {0.2990f,  0.5000f,  0.8660f, 0.0f},
            {0.5870f,  0.5000f, -0.8660f, 0.0f},
            {0.1140f, -1.0000f,  0.0000f, 0.0f},
            {0.0000f,  0.0000f,  0.0000f, 1.0f}
        }
    };

    Vector v = {color.r, color.g, color.b};
    v = matrix_mul(m, v);

    float l = v.x;
    float c1 = v.y;
    float c2 = v.z;

    float theta = atan2(c2, c1);
    float theta_o = (fabs(theta) < M_PI/3)
        ? theta * 3.0/2.0
        : M_PI/2 + (3.0/4.0) * (theta - M_PI/3); /* XXX handle theta_o < 0 */

    float c = cos(theta_o - theta);
    float s = sin(theta_o - theta);

    float cyb = c * c1 - s * c2;
    float crg = s * c1 - c * c2;

    l_cyb_crg[0] = l;
    l_cyb_crg[1] = cyb;
    l_cyb_crg[2] = crg;
}

Color color_from_orgb(float l, float cyb, float crg)
{
    static Matrix const m =
    {
        {
            {1.0000,  1.0000,  1.0000, 0.0},
            {0.1140,  0.1140, -0.8660, 0.0},
            {0.7436, -0.4111,  0.1663, 0.0},
            {0.0000,  0.0000,  0.0000, 1.0}
        }
    };

#if 1
    float theta_o = atan2(crg, cyb);
    float theta = 0;
    if (fabs(theta_o) < M_PI/2)
    {
        theta = theta_o * 2.0/3.0;
    }
    else if (theta_o > 0)
    {
        theta = +M_PI/3 + (4.0/3.0) * (theta_o - M_PI/2);
    }
    else
    {
        theta = -M_PI/3 + (4.0/3.0) * (theta_o + M_PI/2);
    }

    float c = cos(theta - theta_o);
    float s = sin(theta - theta_o);

    float c1 = c * cyb - s * crg;
    float c2 = s * cyb + c * crg;
#else
    float c1 = cyb;
    float c2 = crg;
#endif

    Vector v = {l, c1, c2};
    v = matrix_mul(m, v);

    return color_from_rgb(v.x, v.y, v.z);
}

Color color_wheel_hsv(float phi)
{ 
    return color_from_hsv(phi * 180 / M_PI, 1, 1);
}

Color color_wheel_orgb(float phi)
{
    float x = cos(phi);
    float y = sin(phi);
#if 0
    float norm = fmax(fabs(x), fabs(y));
#else
    float norm = 1;
#endif

    float cyb = x / norm;
    float crg = y / norm;

    return color_from_orgb(0.5, cyb, crg);
}

Color color_interpolate(Color from, Color to, float t)
{
    Color c =
    {
        interpolate(from.r, to.r, t),
        interpolate(from.g, to.g, t),
        interpolate(from.b, to.b, t)
    };

    return c;
}

Color4 color4_interpolate(Color4 from, Color4 to, float t)
{
    Color4 c =
    {
        color_interpolate(from.c, to.c, t),
        interpolate(from.a, to.a, t)
    };

    return c;
}

int color_from_string(char const string[], Color * color)
{
    return sscanf(string, "color(%f,%f,%f)", &color->r, &color->g, &color->b) == 3;
}

int color_from_rgb_string(char const string[], Color * color)
{
    int r, g, b;

    int matches = sscanf(string, "rgb(%d,%d,%d)", &r, &g, &b);
    if (matches == 3)
    {
        *color = color_from_rgb(r / 255.0, g / 255.0, b / 255.0);
        return 1;
    }

    return 0;
}

int color_from_hex_string(char const string[], Color * color)
{
    unsigned r, g, b;

    int matches = sscanf(string, "#%2x%2x%2x", &r, &g, &b);
    if (matches != 3)
        return 0;

    *color = color_from_rgb(r / 255.0, g / 255.0, b / 255.0);

    return 1;
}

Color color_from_value(unsigned long value)
{
    int r = (value & 0xff0000) >> 16;
    int g = (value & 0x00ff00) >> 8;
    int b = (value & 0x0000ff);

    return color_from_rgb(r / 255.0, g / 255.0, b / 255.0);
}


Color color_scale(Color color, float factor)
{
    color.r *= factor;
    color.g *= factor;
    color.b *= factor;

    return color;
}

Color color_mul(Color a, Color b)
{
    a.r *= b.r;
    a.g *= b.g;
    a.b *= b.b;

    return a;
}

Color color_multiply(Color color, Color light, float factor)
{
    color.r *= light.r * factor;
    color.g *= light.g * factor;
    color.b *= light.b * factor;

    return color;
}

Color color_div(Color c1, Color c2)
{
    c1.r /= c2.r;
    c1.g /= c2.g;
    c1.b /= c2.b;

    return c1;
}

Color color_add(Color a, Color b)
{
    Color c =
    {
        a.r + b.r,
        a.g + b.g,
        a.b + b.b
    };

    return c;
}

Color color_add_3(Color a, Color b, Color c)
{
    Color d =
    {
        a.r + b.r + c.r,
        a.g + b.g + c.g,
        a.b + b.b + c.b
    };

    return d;
}

Color color_sub(Color a, Color b)
{
    Color c =
    {
        a.r - b.r,
        a.g - b.g,
        a.b - b.b
    };

    return c;
}

Color color_pow(Color c, float e)
{
    c.r = pow(c.r, e);
    c.g = pow(c.g, e);
    c.b = pow(c.b, e);

    return c;
}

Color color_log(Color c)
{
    c.r = log(c.r + 0.0000001);
    c.g = log(c.g + 0.0000001);
    c.b = log(c.b + 0.0000001);

    return c;
}

Color color_abs(Color c)
{
    c.r = fabsf(c.r);
    c.g = fabsf(c.g);
    c.b = fabsf(c.b);

    return c;
}

Color color_sqrt(Color c)
{
    c.r = sqrt(c.r);
    c.g = sqrt(c.g);
    c.b = sqrt(c.b);

    return c;
}

int color_similar(Color a, Color b, float tolerance)
{
    Color c = color_sub(a, b);

    return
        fabs(c.r) <= tolerance &&
        fabs(c.g) <= tolerance &&
        fabs(c.b) <= tolerance;
}

void color_accumulate(Color * accumulator, Color color, float scale)
{
#ifdef USE_FMA
    accumulator->r = fmaf(color.r, scale, accumulator->r);
    accumulator->g = fmaf(color.g, scale, accumulator->g);
    accumulator->b = fmaf(color.b, scale, accumulator->b);
#else
    accumulator->r += color.r * scale;
    accumulator->g += color.g * scale;
    accumulator->b += color.b * scale;
#endif
}

void color_accumulate2(Color * accumulator, Color color_1, Color color_2, float scale)
{
    accumulator->r += color_1.r * color_2.r * scale;
    accumulator->g += color_1.g * color_2.g * scale;
    accumulator->b += color_1.b * color_2.b * scale;
}

Color color_clamp(Color color)
{
    if (color.r > 1) color.r = 1;
    if (color.g > 1) color.g = 1;
    if (color.b > 1) color.b = 1;

    return color;
}

Color color_clamp_max(Color color, Color max_color)
{
    if (color.r > max_color.r) color.r = max_color.r;
    if (color.g > max_color.g) color.g = max_color.g;
    if (color.b > max_color.b) color.b = max_color.b;

    return color;
}

Color color_clamp_min(Color color, Color min_color)
{
    if (color.r < min_color.r) color.r = min_color.r;
    if (color.g < min_color.g) color.g = min_color.g;
    if (color.b < min_color.b) color.b = min_color.b;

    return color;
}

Color color_invert(Color color)
{
    color.r = 1 - color.r;
    color.g = 1 - color.g;
    color.b = 1 - color.b;

    return color;
}

float color_distance(Color a, Color b)
{
    Vector delta =
    {
        a.r - b.r,
        a.g - b.g,
        a.b - b.b
    };

    return vector_length(delta);
}

static float tone_map(float value, float white)
{
    return value * (1 + value / (white * white)) / (1 + value);
}

Color color_tone_map(Color color, Color white)
{
    color.r = tone_map(color.r, white.r);
    color.g = tone_map(color.g, white.g);
    color.b = tone_map(color.b, white.b);

    return color;
}

Color color_interpolate_bilinear(Color const colors[2][2], float u, float v)
{
    Color c0 = color_interpolate(colors[0][0], colors[0][1], u);
    Color c1 = color_interpolate(colors[1][0], colors[1][1], u);

    return color_interpolate(c0, c1, v);
}

Color4 color4_interpolate_bilinear(Color4 const colors[2][2], float u, float v)
{
    Color4 c0 = color4_interpolate(colors[0][0], colors[0][1], u);
    Color4 c1 = color4_interpolate(colors[1][0], colors[1][1], u);

    return color4_interpolate(c0, c1, v);
}

#if 0
#ifdef Complex
#undef Complex
#endif

Color color_from_complex(Complex z)
{
    float radius = complex_radius(z);
    float phi = complex_arg(z);

    return color_from_hsv(phi, 1 / (1 + radius), 1);
}
#endif

static Color color_from_vector(Vector v)
{
    Color c = {v.x, v.y, v.z};
    return c;
}

static Vector color_to_vector(Color c)
{
    Vector v = {c.r, c.g, c.b};
    return v;
}

Vector color_xyz_to_lms(Vector v)
{
    return matrix_mul(XYZ_TO_LMS, v);
}

Vector color_lms_to_xyz(Vector v)
{
    return matrix_mul(matrix_invert(XYZ_TO_LMS), v);
}

Color color_from_yuv(Vector v)
{
    v = matrix_mul(YUV_TO_RGB, v);
    return color_from_vector(v);
}

Vector color_to_yuv(Color c)
{
    Vector v = color_to_vector(c);
    return matrix_mul(RGB_TO_YUV, v);
}

Color color_from_xyz(Vector v)
{
    v = matrix_mul(matrix_invert(RGB_TO_XYZ), v);
    return color_from_vector(v);
}

Vector color_to_xyz(Color c)
{
    Vector v = color_to_vector(c);
    return matrix_mul(RGB_TO_XYZ, v);
}

Color color_from_srgb(Color c)
{
    c.r = gamma_correct(c.r, GAMMA);
    c.g = gamma_correct(c.g, GAMMA);
    c.b = gamma_correct(c.b, GAMMA);

    return c;
}

Color color_to_srgb(Color c)
{
    float const INV_GAMMA = 1.0 / GAMMA;

    c.r = gamma_correct(c.r, INV_GAMMA);
    c.g = gamma_correct(c.g, INV_GAMMA);
    c.b = gamma_correct(c.b, INV_GAMMA);

    return c;
}

static float f(float t)
{
    float const delta = 6.f / 29;
    float const delta_2 = delta * delta;
    float const delta_3 = delta * delta * delta;

    return t > (delta_3)
        ? pow(t, (float) (1.0/3.0))
        : delta_2 * t / 3.0 + 4 / 29.0;
}

Vector color_xyz_to_lab(Vector v, Vector n)
{
    float L = 116 * f(v.y / n.y) - 16;
    float a = 500 * (f(v.x / n.x) - f(v.y / n.y));
    float b = 200 * (f(v.y / n.y) - f(v.z / n.z));

    Vector result = {L, a, b};
    return result;
}

Vector color_lab_to_xyz(float L, float a, float b, Vector n)
{
    float const delta = 6 / 29.0;
    float const delta_2 = delta * delta;

    Vector f;
    f.y = (L + 16) / 116;
    f.x = f.y + a / 500;
    f.z = f.y - b / 200;

    Vector v;
    if (f.x > delta)
        v.x = n.x * f.x;
    else
        v.x = (f.x - 16 / 116.0) * 3 * delta_2 * n.x;

    if (f.y > delta)
        v.y = n.y * f.y;
    else
        v.y = (f.y - 16 / 116.0) * 3 * delta_2 * n.y;

    if (f.z > delta)
        v.z = n.z * f.z;
    else
        v.z = (f.z - 16 / 116.0) * 3 * delta_2 * n.z;

    return v;
}

int color_is_negative(Color c)
{
    return c.r < 0 || c.g < 0 || c.b < 0;
}

int color_is_nan(Color c)
{
#ifdef WINDOWS
    return 0; // XXX
#else
    return
        isnan(c.r) ||
        isnan(c.g) ||
        isnan(c.b);
#endif
}

int color_is_inf(Color c)
{
    return
        isinf(c.r) ||
        isinf(c.g) ||
        isinf(c.b);
}

int color_is_bounded(Color c)
{
    return
        (c.r >= 0.0 && c.r <= 1.0) &&
        (c.g >= 0.0 && c.g <= 1.0) &&
        (c.b >= 0.0 && c.b <= 1.0);
}

int color_is_valid(Color c)
{
    return ! color_is_negative(c) && ! color_is_nan(c) && ! color_is_inf(c);
}

int color_is_black(Color c)
{
    return c.r == 0.0 && c.g == 0.0 && c.b == 0.0;
}

Color color_negate(Color c)
{
    c.r = 1.0 - c.r;
    c.g = 1.0 - c.g;
    c.b = 1.0 - c.b;

    return c;
}

Color color_square(Color c)
{
    c.r = c.r * c.r;
    c.g = c.g * c.g;
    c.b = c.b * c.b;

    return c;
}

Color color_min(Color c1, Color c2)
{
    Color c =
    {
        fmin(c1.r, c2.r),
        fmin(c1.g, c2.g),
        fmin(c1.b, c2.b)
    };

    return c;
}

Color color_max(Color c1, Color c2)
{
    Color c =
    {
        fmax(c1.r, c2.r),
        fmax(c1.g, c2.g),
        fmax(c1.b, c2.b)
    };

    return c;
}

int color_parser(void const * data, char const string[], void * target)
{
    return
        color_from_string(string, (Color *) target) ||
        color_from_rgb_string(string, (Color *) target) ||
        color_from_hex_string(string, (Color *) target) ||
        color_from_svg_name(string,   (Color *) target);
}

int color_printer(void const * data, void const * source, char string[])
{
    Color color = * (Color *) source;
    return sprintf(string, "rgb(%g, %g, %g)", color.r, color.g, color.b);
}

float color_average(Color c)
{
    return (c.r + c.g + c.b) / 3.0;
}

Color color_normalize(Color c)
{
    float length = sqrtf(c.r * c.r + c.g * c.g + c.b * c.b);
    float factor = 1.0 / length;

    c.r *= factor;
    c.g *= factor;
    c.b *= factor;

    return c;
}

void color_patch_reflectance(Color * c, float wave_length)
{
    if (wave_length < 475) // blue
        c->r = c->g = c->b;
    else if (wave_length < 510) // green
    {
        float t = normalize(475, 510, wave_length);
        c->r = c->g = c->b = interpolate(c->b, c->g, t);
    }
    else if (wave_length < 650) // red
    {
        float t = normalize(510, 650, wave_length);
        c->r = c->g = c->b = interpolate(c->g, c->r, t);
    }
    else
        c->g = c->b = c->r;
}

Color color_add_scaled(Color c1, Color c2, float scale)
{
    c1.r += c2.r * scale;
    c1.g += c2.g * scale;
    c1.b += c2.b * scale;

    return c1;
}

Color4 color4_add_scaled(Color4 c1, Color4 c2, float scale)
{
    color_accumulate(&c1.c, c2.c, scale);
    Color4 c = {c1.c, 1};
    return c;
}

Color color_correct_gamma(Color color, float gamma)
{
    color.r = gamma_correct(color.r, gamma);
    color.g = gamma_correct(color.g, gamma);
    color.b = gamma_correct(color.b, gamma);

    return color;
}

Color color_update_sample_mean(Color mean_n, Color x_n1, int n)
{
    return color_scale(color_add(color_scale(mean_n, n), x_n1), 1.0 / (n + 1));
}

Color color_update_sample_variance(Color var_n, Color mean_n1, Color x_n1, int n)
{
    if (n == 0)
        return BLACK;

    Color a = color_scale(var_n, (float) n / (n + 1));
    Color b = color_scale(color_square(color_sub(x_n1, mean_n1)), 1.0 / n);
    return color_add(a, b);
}

Color color_blend(Color c1, float f1, Color c2, float f2)
{
    Color c =
    {
        f1 * c1.r + f2 * c2.r,
        f1 * c1.g + f2 * c2.g,
        f1 * c1.b + f2 * c2.b
    };

    return c;
}

Color_RGBE color_to_rgbe(Color c)
{
    float max_amplitude = fmax(c.r, fmax(c.g, c.b));

    int exponent = ceil(log(max_amplitude) / log(2.0));
    float amplitude = pow(2.0, exponent);

    Color_RGBE rgbe =
    {
        (unsigned char) (255 * c.r / amplitude),
        (unsigned char) (255 * c.g / amplitude),
        (unsigned char) (255 * c.b / amplitude),
        exponent
    };

    return rgbe;
}

Color color_from_rgbe(Color_RGBE rgbe)
{
    float amplitude = pow(2.0, rgbe.e);
    float scale = amplitude / 255.0;

    Color c =
    {
        rgbe.r * scale,
        rgbe.g * scale,
        rgbe.b * scale
    };

    return c;
}

