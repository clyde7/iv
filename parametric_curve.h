#ifndef PARAMETRIC_CURVE_H
#define PARAMETRIC_CURVE_H

#include "vector.h"

typedef Vector (* Parametric_Curve)(void const *, float);

typedef struct {int p, q;} Torus_Knot;
extern Torus_Knot const TREEFOIL_KNOT;
Vector torus_knot(void const *, float);

Vector heart(void const *, float);

#endif
