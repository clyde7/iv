#ifndef TEXTURE_H
#define TEXTURE_H

#include "color.h"
#include "image.h"
#include "vector.h"
#include "volume.h"

typedef Color (* Texture)(void *, Vector);

Color texture_checker(Vector);
void  texture_download(Image const *);
void  texture_download_target(Image const *, GLenum target);
void  texture_download_layer(Image const *, int layer);

Brick * brick_from_image(Image const *);

float luminance_overcast_sky(Vector omega);
float luminance_clear_sky(Vector omega, Vector sun);
float luminance_clear_sky_simple(Vector omega, Vector sun);
float luminance_head(Vector omega, float cut_off_angle);

#endif
