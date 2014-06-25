#include "opengl.h"
#include "pixel_transfer.h"

void pixel_transfer_scale_alpha(Color color, float alpha)
{
    glPixelTransferf(GL_RED_SCALE, color.r);
    glPixelTransferf(GL_GREEN_SCALE, color.g);
    glPixelTransferf(GL_BLUE_SCALE, color.b);
    glPixelTransferf(GL_ALPHA_SCALE, alpha);
}

void pixel_transfer_scale(Color color)
{
    pixel_transfer_scale_alpha(color, 1.0);
}

void pixel_transfer_bias_alpha(Color color, float alpha)
{
    glPixelTransferf(GL_RED_BIAS, color.r);
    glPixelTransferf(GL_GREEN_BIAS, color.g);
    glPixelTransferf(GL_BLUE_BIAS, color.b);
    glPixelTransferf(GL_ALPHA_BIAS, alpha);
}

void pixel_transfer_bias(Color color)
{
    pixel_transfer_bias_alpha(color, 0.0);
}

void pixel_transfer_reset(void)
{
    static Color const ONE = {1, 1, 1};
    static Color const ZERO = {0, 0, 0};

    pixel_transfer_scale_alpha(ONE, 1.0);
    pixel_transfer_bias_alpha(ZERO, 0.0);
}
