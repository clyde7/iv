#ifndef COLOR_H
#define COLOR_H

#include "vector.h"

#define rgb(r, g, b) (color_from_rgb((r), (g), (b)))
#define rgb8(r, g, b) (color_from_rgb((r)/255.0, (g)/255.0, (b)/255.0))

typedef struct {float r, g, b;} Color;
typedef struct {Color c; float a;} Color4;
typedef struct {float wave_length, r, g, b;} Spectral_Color;
typedef struct {unsigned char r, g, b, e;} Color_RGBE;

typedef Color (* Color_Map)(unsigned);

extern Color const BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, GRAY, ORANGE, VIOLET, MIN_COLOR, MAX_COLOR;

void color_print(Color);
void color_print_pattern(Color, char const pattern[]);
void color_print_name(Color);
void color_apply(Color);
void color4_apply(Color4);
void color_apply_alpha(Color, float alpha);
void color_table(Color_Map, unsigned char table[256][3]);
void color_table_transposed(Color_Map, unsigned char table[3][256]);
void color_to_cmyk(Color, float cmyk[4]);
void color_to_orgb(Color, float l_cyb_crg[3]);

Color_RGBE color_to_rgbe(Color);
Color color_from_rgbe(Color_RGBE);

Color4 color4(Color, float);
Vector color_xyz_to_lab(Vector v, Vector n);
Vector color_lab_to_xyz(float L, float a, float b, Vector n);
Vector color_xyz_to_lms(Vector);
Vector color_lms_to_xyz(Vector);
Color  color_from_yuv(Vector);
Vector color_to_yuv(Color);
Color  color_from_xyz(Vector);
Vector color_to_xyz(Color);
Color color_from_srgb(Color);
Color color_to_srgb(Color);
Color color_from_rgb(float red, float green, float blue);
Color color_from_cmyk(float const cmyk[4]);
Color color_from_hsv(float hue, float saturation, float value);
void  color_to_hsv(Color c, float * hue, float * saturation, float * value);
Color color_from_orgb(float l, float cyb, float crg);
Color color_from_value(unsigned long);
Color color_wheel_hsv(float phi);
Color color_wheel_orgb(float phi);
Color color_interpolate(Color from, Color to, float t);
Color4 color4_interpolate(Color4 from, Color4 to, float t);
Color color_scale(Color, float);
Color color_mul(Color, Color);
Color color_multiply(Color, Color, float);
Color color_div(Color, Color);
Color color_add(Color, Color);
Color color_add_3(Color, Color, Color);
Color color_add_scaled(Color, Color, float);
Color color_sub(Color, Color);
Color color_pow(Color, float);
Color color_log(Color);
Color color_abs(Color);
Color color_sqrt(Color);
int   color_similar(Color, Color, float tolerance);
Color color_clamp(Color);
Color color_clamp_max(Color, Color);
Color color_clamp_min(Color, Color);
Color color_invert(Color);
void  color_accumulate(Color *, Color, float);
void  color_accumulate2(Color *, Color, Color, float);
Color color_update_sample_mean(Color mean_n, Color x_n1, int n);
Color color_update_sample_variance(Color var_n, Color mean_n1, Color x_n1, int n);
Color color_tone_map(Color, Color white);
float color_distance(Color, Color);
int   color_is_negative(Color);
int   color_is_nan(Color);
int   color_is_inf(Color);
int   color_is_valid(Color);
int   color_is_bounded(Color);
int   color_is_black(Color);
Color color_negate(Color);
Color color_square(Color);
Color color_min(Color, Color);
Color color_max(Color, Color);
float color_average(Color);
Color color_normalize(Color);
void color_patch_reflectance(Color *, float wave_length);

int color_from_string(char const string[], Color *);
int color_from_rgb_string(char const string[], Color *);
int color_from_hex_string(char const string[], Color *);
int color_from_svg_name(char const name[], Color *);
int color_from_c64_name(char const name[], Color *);
Color color_from_name(char const name[]);
char const * color_to_svg_name(Color);
char const * color_to_c64_name(Color);

Color color_interpolate_bilinear(Color const colors[2][2], float u, float v);
Color4 color4_interpolate_bilinear(Color4 const colors[2][2], float u, float v);

Color4 color4_add_scaled(Color4, Color4, float);

int color_parser(void const *, char const string[], void * target);
int color_printer(void const *, void const * source, char string[]);

Color color_correct_gamma(Color, float);
Color color_blend(Color, float, Color, float);

#endif
