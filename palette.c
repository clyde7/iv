#include "color.h"
#include "math_.h"
#include "palette.h"

Image * palette_petroleum(int size)
{
    Image_Format format = {GL_FLOAT, GL_RGB, {size, 1, 1}};
    Image * palette = image_new(format);
    Color * target = (Color *) palette->pixels;

    for (int i = 0; i != size; ++ i)
    {
        float phi = i * 2.0 * M_PI / (size - 1);

        Color c =
        { 
            (1 + cos(phi)) / 2,
            (1 + sin(phi)) / 2,
            (1 - cos(phi)) / 2
        };

        * target ++ = c; 
    }

    return palette;
}

Image * palette_false(int size)
{
    Image_Format format = {GL_FLOAT, GL_RGB, {size, 1, 1}};
    Image * palette = image_new(format);
    Color * target = (Color *) palette->pixels;

    for (int i = 0; i != size; ++ i)
    {
        float t = (float) i / (size - 1);
        float phi = interpolate(0, 1.5 * M_PI, t) - M_PI;

        * target ++ = color_wheel_orgb(phi);
    }

    return palette;
}

Image * palette_transform(Image const * image)
{
    Size size = {image->format.size.x, 4, 1};
    Image_Format format = {GL_FLOAT, GL_LUMINANCE, size};
    Image * palette = image_new(format);

    float const * source = (float const *) image->pixels;
    float * target = (float *) palette->pixels;

    for (int i = 0; i != size.y; ++ i)
    for (int j = 0; j != size.x; ++ j)
    {
        int source_index = size_index(image->format.size, 0, 0, j);
        int target_index = size_index(format.size, 0, i, j);

        target[target_index] = source[source_index + i];
    }

    return palette;
}
