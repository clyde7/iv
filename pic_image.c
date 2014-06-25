#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "error.h"
#include "image.h"

void pic_save(Image const * image, char const filename[])
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "image type must be float");
    error_check(format.format != GL_RGB, "image type must be rgb");

    int width  = format.size.x;
    int height = format.size.y;

    FILE * file = fopen(filename, "wb");
    fprintf(file, "#?RADIANCE\n");
    fprintf(file, "FORMAT=32-bit_rle_rgbe\n");
    fprintf(file, "\n");
    fprintf(file, "+Y %d +X %d\n", height, width);

    Color const * pixels = (Color const *) image->pixels;
    int pixel_count = width * height;

    for (int i = 0; i != pixel_count; ++ i)
    {
        Color color = pixels[i];
        Color_RGBE c = color_to_rgbe(color);

        fwrite(&c, sizeof(unsigned char), 4, file);
    }

    fclose(file);
}

Image * pic_load(FILE * file)
{
    int x, y, width, height;

    //FILE * file = fopen(filename, "rb");
    fscanf(file, "#?RADIANCE\n");
    fscanf(file, "\n");
    fscanf(file, "%d %d %d %d", &y, &height, &x, &width);

    int pixel_count = width * height;

    //int flip_x = x < 0;
    int flip_y = y < 0;

    x = abs(x);
    y = abs(y);

    Image_Format format = {GL_FLOAT, GL_RGB, {width, height, 1}};
    Image * image = image_new(format);
    Color * pixels = (Color *) image->pixels;

    for (int i = 0; i != pixel_count; ++ i)
    {
        Color_RGBE rgbe;
        fread(&rgbe, 1, 4, file);

        * pixels ++ = color_from_rgbe(rgbe);
    }

    if (flip_y)
        image_flip(image);

    return image;
}

