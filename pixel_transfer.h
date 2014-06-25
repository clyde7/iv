#ifndef PIXEL_TRANSFER_H
#define PIXEL_TRANSFER_H

#include "color.h"

void pixel_transfer_scale(Color);
void pixel_transfer_scale_alpha(Color, float);
void pixel_transfer_bias(Color);
void pixel_transfer_bias_alpha(Color, float);
void pixel_transfer_reset(void);

#endif
