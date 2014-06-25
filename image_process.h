#ifndef IMAGE_PROCESS_H
#define IMAGE_PROCESS_H

#include "box.h"
#include "color.h"
#include "image.h"
#include "math_.h"
#include "texture.h"

Image * image_retype(Image const *, GLenum target_type);
Image * image_reformat(Image const *);
Image * image_rgb_to_gray(Image const *);
Image * image_extract(Image const *, GLenum target_format, GLenum source_component, GLenum target_component);
Image * image_pack(Image * red, Image * green, Image * blue, Image * alpha);
Image * image_normals(Image const *, float scale);
void  image_remap(Image *, float scale, float bias);
void  image_normalize(Image *);
void  image_convert(Image *, Image_Format);
void  image_add_alpha(Image const *, Image *);
void  image_remove_alpha(Image const *, Image *);

void  image_resize(Image const *, Image *);
void  image_fill(Image *, Texture, void *, Box);
void  image_fill_color(Image *, Texture, void *, Box);
void  image_gradient(Image *, Color const values[2][2]);
unsigned short image_sample_index(Image const *, Vector, Border);
Color image_sample(Image const *, Vector, Border);
float image_sample_1D(Image const *, float);
float image_sample_2D(Image const *, Vector);
Color image_sample_color_2D(Image const *, Vector, Border);
Color4 image_sample_color4_2D(Image const *, Vector, Border);
Image * image_histogram(Image const *, int bin_count);
void  image_min_max(Image const *, float * min, float * max);

Image * image_convolve(Image const *, float const weights[], int size, Border);
Image * image_fast_area_sum(Image const *);

void image_correct_gamma(Image *, float);
void image_negate(Image *);

typedef struct {Color mean, var, min, max;} Image_Statistics;
typedef struct {int neg, inf, nan;} Image_Problems;

void             image_statistics_print(Image_Statistics);
float            image_statistics_threshold(Image const * image, float threshold);
Image_Statistics image_statistics(Image const *);

void           image_problems_print(Image_Problems);
Image_Problems image_problems(Image const *);

Image * cloud_image(int size, float r, float mean, float H, int index);
Image * cloud_image_3d(int size, float roughness, float mean, float H);

Image * image_bump_map(Image const * image, float scale);

Image * image_apply_palette(Image const *, Image const *);
void    image_accumulate(Image * accumulator, Image const *, float);
void    image_accumulate_convergence(Image * target, Image const * reference, Image const * source, float tolerance, int iteration);
void    image_scale(Image *, float);
void    image_log(Image *);

Image * image_diff(Image const * image1, Image const * image2);
Image * image_crop(Image const *, Size position, Size size);
Image * image_pad(Image const *, Size padding);
Image * image_pad_border(Image const *, Size padding, Border);
Image * image_cut(Image const *, Size position, Size size, Border);
void image_cut_raw(Image const *, Size position, Size size, Border, Image *);

void    image_fill_all(Image *, Color4);
void    image_fill_rectangle(Image *, Size position, Size size, Color4);
void    image_copy_rectangle(Image *, Image const *, Size target_position, Size source_position, Size);
void    image_paste(Image *, Image const *, Size target_position);
Image * image_zoom(Image const *, Size scale);
void    image_flip_horizontal(Image * image);

int     image_count_zeros(Image const *);
int     image_sum_values(Image const *);
void    image_accumulate_masked(Image * target, Image const * mask, Image const * source, float scale);
void    image_assign_divide(Image *, Image const * denominator_image, unsigned short zero_denominator);

Image * image_blend(Image const * image_1, float t1, Image const * image_2, float t2);
#define image_interpolate(image_1, image_2, t) image_blend((image_1), 1 - (t), (image_2), (t))
#define image_subtract(image_1, image_2)       image_blend((image_1), 1, (image_2), -1)
#define image_add(image_1, image_2)            image_blend((image_1), 1, (image_2), 1)
void    image_assign_square(Image *);
Image * image_divide(Image const *, Image const *, float clamp_value);
void    image_clean_nan(Image *);

Image * image_merge(Image const * const *, int count);
Image * image_duplicate_layers(Image const *, int layer_count);

Image * image_squared_error(Image const * sources, Image const * reference, int relative);
//Image * image_ssim(Image const * sources, Image const * reference);
//float image_mssim(Image const * sources, Image const * reference);
//float image_dmssim(float mssim);
float image_mse(Image const * sources, Image const * reference, int relative);
float image_rmse(float mse);
float image_psnr(float mse);
#define PSNR(reference, other) (image_psnr(image_mse((other), (reference), 0)))

Image * image_squared_error_visual(Image const * sources, Image const * reference);
Image * image_laplacian_error(Image const * sources, Image const * reference, int relative);
Image * image_select_bandwidth(Image const * error);
Image * image_filter(Image const * source, Image const * selected_bandwidths);

Color image_mssim(Image const *, Image const *, float L, float k_1, float k_2, float alpha, float beta, float gamma);
float image_similar(Image const * reference, Image const * image, float tolerance);
float * image_error_distribution(Image const *, int n, int logarithmic);

Image * image_to_lab(Image const *);
void  image_splat(Image * target, Image const * source, Image const * kernel, int rescale);

void image_update_mean    (Image * mean_n, Image const * x_n1, int n);
void image_update_variance(Image *  var_n, Image const * mean_n1, Image const * x_n1, int n);

Image * image_filter_median(Image const *, int width, int median_index);
Image * image_filter_median_weighted(Image const * source);

void image_add_procedural(Image *, float (*) (void *), void *);
void image_add_noise_uniform(Image *, float a, float b);
void image_add_noise_normal(Image *, float mu, float sigma);
void image_add_noise(Image *, float sigma);

void image_add_2(Image const *, Image const *, Image *);

void image_yuv_to_rgb(Image *);
void image_rgb_to_yuv(Image *);

#endif
