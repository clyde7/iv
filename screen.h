#ifndef SCREEN_H
#define SCREEN_H

#include "matrix.h"

typedef struct {float phi_min, phi_max, theta_min, theta_max;} Screen_;

extern Screen_ const DEFAULT_SCREEN;

void   screen_print(Screen_);
Matrix screen_to_projection(Screen_);
Matrix screen_to_projection2(Screen_);
Matrix screen_to_projection3(Screen_);

#endif
