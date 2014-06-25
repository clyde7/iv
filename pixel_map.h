#ifndef PIXEL_MAP_H
#define PIXEL_MAP_H

#include "image.h"

void pixel_map_apply(Image const *);
void pixel_map_correct_gamma(float);
void pixel_map_reset(void);

#endif
