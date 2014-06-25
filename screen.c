#include <stdio.h>

#include "math_.h"
#include "screen.h"

Screen_ const DEFAULT_SCREEN = {-M_PI/4, M_PI/4, -M_PI/4, M_PI/4};

void screen_print(Screen_ screen)
{
    printf("phi = [%f, %f], theta = [%f, %f]", screen.phi_min, screen.phi_max, screen.theta_min, screen.theta_max);
}

Matrix screen_to_projection(Screen_ screen)
{
    float
        n = 0.1,
        //f = 600,
        f = 21,
        l = n * tan(screen.phi_min),
        r = n * tan(screen.phi_max),
        b = n * tan(screen.theta_min),
        t = n * tan(screen.theta_max);

    return matrix_frustum(l, r, b, t, n, f);
}

Matrix screen_to_projection2(Screen_ screen)
{
    float
        n = 0.1,
        f = 600,
        //f = 21,
        l = 1 * tan(screen.phi_min),
        r = 1 * tan(screen.phi_max),
        b = 1 * tan(screen.theta_min),
        t = 1 * tan(screen.theta_max);

    return matrix_ortho(l, r, b, t, n, f);
}

Matrix screen_to_projection3(Screen_ screen)
{
    float
        n = 0.1,
        f = 1500,
        l = 1 * tan(screen.phi_min),
        r = 1 * tan(screen.phi_max),
        b = 1 * tan(screen.theta_min),
        t = 1 * tan(screen.theta_max);

    return matrix_frustum(l, r, b, t, n, f);
}
