#include "error.h"
#include "image_process.h"
#include "kernel.h"
#include "math_.h"

Image * kernel_gaussian_2d(int width, float sigma, Vector sample)
{
    error_check((width % 2) != 1, "kernel width must be odd");

    float sigma2 = SQ(sigma);
    int radius = width / 2;

    Image_Format format = {GL_FLOAT, GL_LUMINANCE, {width, width, 1}};
    Image * image = image_new(format);

    float * pixels = (float *) image->pixels;
    Size size = format.size;

    float accumulator = 0.0;

    for (int i = 0; i != width; ++ i)
    for (int j = 0; j != width; ++ j)
    {
        int index = size_index(size, 0, i, j);
        float dy = i - radius - sample.y;
        float dx = j - radius - sample.x;

        float weight = gaussian_2d(sigma2, SQ(dx) + SQ(dy));

        pixels[index] = weight;
        accumulator += weight;
    }

    image_scale(image, 1.0 / accumulator);

    return image;
}

Image * kernel_box_2d(int width, Vector sample)
{
    Image_Format format = {GL_FLOAT, GL_LUMINANCE, {width, width, 1}};
    Image * image = image_new(format);

    float * pixels = (float *) image->pixels;
    Size size = format.size;

    float weight = 1.0 / (width * width);

    for (int i = 0; i != width; ++ i)
    for (int j = 0; j != width; ++ j)
    {
        int index = size_index(size, 0, i, j);
        pixels[index] = weight;
    }

    return image;
}

Image * kernel_laplacian(void)
{
    Image_Format format = {GL_FLOAT, GL_LUMINANCE, {3, 3, 1}};
    Image * image = image_new(format);
    float * pixels = (float *) image->pixels;

    pixels[0] = 0; pixels[1] = 1; pixels[2] = 0; 
    pixels[3] = 1; pixels[4] =-4; pixels[5] = 1; 
    pixels[6] = 0; pixels[7] = 1; pixels[8] = 0; 

    return image;
}

Image * kernel_laplacian_2(void)
{
    Image_Format format = {GL_FLOAT, GL_LUMINANCE, {3, 3, 1}};
    Image * image = image_new(format);
    float * pixels = (float *) image->pixels;

    pixels[0] = 1; pixels[1] = 1; pixels[2] = 1; 
    pixels[3] = 1; pixels[4] =-8; pixels[5] = 1; 
    pixels[6] = 1; pixels[7] = 1; pixels[8] = 1; 

    return image;
}

float kernel_pixel_filter_energy(Image const * kernel)
{
    float const * weights = (float const *) kernel->pixels;
    Size size = kernel->format.size;

    float accumulator = 0.0;

    for (int i = 0; i != size.y; ++ i)
    for (int j = 0; j != size.x; ++ j)
    {
        int source_index_1 = size_index(size, 0, i, j);

        for (int k = 0; k != size.y; ++ k)
        for (int l = 0; l != size.x; ++ l)
        {
            int source_index_2 = size_index(size, 0, k, l);

            accumulator += weights[source_index_1] * weights[source_index_2];
        }
    }

    return accumulator;
}
