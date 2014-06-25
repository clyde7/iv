#ifndef KERNEL_H
#define KERNEL_H

#include "image.h"

Image * kernel_gaussian_2d(int width, float sigma, Vector sample);
Image * kernel_box_2d(int width, Vector sample);
float   kernel_pixel_filter_energy(Image const * kernel);
Image * kernel_laplacian(void);
Image * kernel_laplacian_2(void);

#endif
