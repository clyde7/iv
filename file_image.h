#ifndef FILE_IMAGE_H
#define	FILE_IMAGE_H

#include <stdio.h>

#include "property.h"
#include "image.h"
#include "vector.h"

Image * bob_load(char const filename[]);

Image * exr_load(char const name[]);
void    exr_save(Image const *, char const name[]);
// TODO first name!
void    exr_save_with_properties(Image const *, char const name[], Property const properties[], int property_count, char const * layer_names[]);
void    exr_load_multi(char const name[], Image ** exv_image, Image ** est_var_image, Image ** emp_var_image, float ** radii, int * radius_count, char kernel_type[]);
void    exr_save_multi(char const name[], Image const * exv_image, Image const * est_var_image, Image const * emp_var_image, Property const properties[], int property_count, float const radii[], int radius_count, char const kernel_type[]);

void exr_save_feature_buffer(char const name[],
    Image const * radiance_exv,
    Image const * radiance_var,
    Image const * weight_exv,
    Image const * weight_var,
    Image const * position_exv,
    Image const * position_var,
    Image const * normal_exv,
    Image const * normal_var,
    Property const properties[], int property_count);

void exr_load_feature_buffer(char const name[],
    Image ** radiance_exv,
    Image ** radiance_var,
    Image ** weight_exv,
    Image ** weight_var,
    Image ** position_exv,
    Image ** position_var,
    Image ** normal_exv,
    Image ** normal_var);

Image * exr_load_layer(char const name[], char const layer_name[]);
Property * exr_image_properties(char const name[], unsigned * count);
int          exr_layer_count(char const file_name[]);
char const * exr_layer_name(char const file_name[], int index);

Image * gif_load(FILE *);
void    gif_save(Image const *, FILE *, int loop_count, float delay);

Image * jpeg_load(FILE *);
void    jpeg_save(Image const *, FILE *);
void    jpeg_snapshot(char const basename[], Viewport, GLenum format);

Image * mpo_load(char const filename[]);

Image * png_load(FILE *);
Image * png_load_flip(FILE *, int flip);
Image_Format png_format(FILE *);
int     png_save(Image const *, FILE *);
int     png_save_with_properties(Image const *, FILE *, Property const properties[], int property_count);
void    png_snapshot_name(char const name[], Viewport, GLenum format, Property const properties[], int property_count);
void    png_snapshot(char const name[], Viewport, GLenum format, Property const properties[], int property_count);
// XXX swap above function names

void    pnm_save(Image const *, char const name[]);
void    pfm_save(Image const *, char const name[]);
void    pgm_save(Image const *, char const name[]);
void    ppm_save(Image const *, char const name[]);
Image * pnm_load(FILE *);
Image * pgm_load(FILE *);
Image * pfm_load(FILE *);
Image * ppm_load(FILE *);

void    pic_save(Image const *, char const name[]);
Image * pic_load(FILE *);

Image_Format raw_format(char const filename[]); // TODO
Image * raw_load(char const filename[], Vector * ratio);
void    raw_save(Image const *, FILE *);

Image * tiff_load(FILE *);

int image_save(Image const *, Property const properties[], int property_count, char const pattern[], char const mime_type[]);
int image_save_basename(Image const *, Property const properties[], int property_count, char const pattern[], char const mime_type[]);

#endif
