#include "parametric_curve.h"

Torus_Knot const TREEFOIL_KNOT = {2, -3};

Vector torus_knot(void const * data, float t)
{
    Torus_Knot const * torus_knot = (Torus_Knot const *) data;
    int p = torus_knot->p;
    int q = torus_knot->q;

    t *= 2 * M_PI;

    Vector v =
    {
        (2 + cos(q * t)) * cos(p * t),
        (2 + cos(q * t)) * sin(p * t),
        sin(q * t)
    };

    return v;
}

Vector heart(void const * data, float t)
{
    t *= 2 * M_PI;

    Vector v =
    {
        16 * pow(sin(t), 3),
        13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t),
        0
    };

    return v;
}

