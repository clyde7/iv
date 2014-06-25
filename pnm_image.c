#include "error.h"
#include "file_image.h"
#include "system.h"
#include "utils.h"

// TODO add binary flag

char const * PPM_MIME = "image/x-ppm";
char const * PGM_MIME = "image/x-pgm";
char const * PNM_MIME = "image/x-portable-anymap";

void pnm_save(Image const * image, char const name[])
{
    if (image->format.type == GL_FLOAT)
        pfm_save(image, name);
    else if (image->format.format == GL_RGB)
        ppm_save(image, name);
    else if (image->format.format == GL_LUMINANCE)
        pgm_save(image, name);
    else
        fprintf(stderr, "pnm_save: unsupported image format\n");
}

void pfm_save(Image const * image, char const name[])
{
    Image_Format format = image->format;
    int const width = format.size.x;
    int const height = format.size.y;

    error_check(format.format != GL_RGB && format.format != GL_LUMINANCE, "only RGB or luminance format supported");
    error_check(format.type != GL_FLOAT, "only float type suppported");

    int channels = format.format == GL_RGB ? 3 : 1;
    int pixel_bytes = channels * sizeof(float);
    float scale = 1.0;

    FILE * file = fopen(name, "wb");
    fprintf(file, "P%c %d %d %f\n", format.format == GL_RGB ? 'F' : 'f', width, height, system_is_big_endian() ? scale : -scale);
    fwrite(image->pixels, pixel_bytes, width * height, file);
    fclose(file);
}

void pgm_save(Image const * image, char const name[])
{
    static int const MAX_VALUE = 255;

    Image_Format format = image->format;
    int const width = format.size.x;
    int const height = format.size.y;
    unsigned char * pixels = (unsigned char *) image->pixels;

    error_check(format.format != GL_LUMINANCE, "only luminance format supported");
    error_check(format.type != GL_UNSIGNED_BYTE, "only unsigned byte type suppported");

    FILE * file = fopen(name, "w");
    fprintf(file, "P2 %d %d %d\n", width, height, MAX_VALUE);

    for (int i = 0; i != height; ++ i)
    {
        for (int j = 0; j != width; ++ j)
        {
            unsigned char value = * pixels ++;

            fprintf(file, "%d ", value);
        }
        fputs("\n", file);
    }

    fclose(file);
}

void ppm_save(Image const * image, char const name[])
{
    static int const MAX_VALUE = 255;

    Image_Format format = image->format;
    int const width = format.size.x;
    int const height = format.size.y;
    unsigned char * pixels = (unsigned char *) image->pixels;

    error_check(format.format != GL_RGB, "only RGB format supported");
    error_check(format.type != GL_UNSIGNED_BYTE, "only unsigned byte type suppported");

    FILE * file = fopen(name, "w");
    fprintf(file, "P3 %d %d %d\n", width, height, MAX_VALUE);

    for (int i = 0; i != height; ++ i)
    {
        for (int j = 0; j != width; ++ j)
        {
            unsigned char red   = * pixels ++;
            unsigned char green = * pixels ++;
            unsigned char blue  = * pixels ++;

            fprintf(file, "%d %d %d ", red, green, blue);
        }
        fputs("\n", file);
    }

    fclose(file);
}

Image * pnm_load(FILE * file)
{
    // XXX hack
    return pnm_load(file);
//    return NULL;
}

Image * pfm_load(FILE * file)
{
    char version;
    int width, height;
    float endian;
    fscanf(file, "P%c %d %d %f\n", &version, &width, &height, &endian);

    // XXX broken

    int same_endian = system_is_big_endian() == (endian == 1.0);

    // TODO channels

    if (version == 'F')
    {
        Image_Format format = {GL_FLOAT, GL_RGB, {width, height, 1}};
        Image * image = image_new(format);
        float * pixels = (float *) image->pixels;

        unsigned char value[4];

        for (int i = 0; i != height; ++ i)
        {
            for (int j = 0; j != width * 3; ++ j)
            {
                fread(&value[0], 1, sizeof(float), file);
                if (! same_endian)
                {
                    swap(char, value[0], value[3]);
                    swap(char, value[1], value[1]);
                }

                * pixels ++ = * (float const *) &value[0];
            }

            fread(&value[0], 1, 1, file);
        }

        fclose(file);

        image_flip(image);

        return image;
    }
    else if (version == 'f')
    {
        Image_Format format = {GL_FLOAT, GL_LUMINANCE, {width, height, 1}};
        Image * image = image_new(format);
        //float * pixels = (float *) image->pixels;

        fclose(file);

        image_flip(image);

        return image;
    }

    return NULL;
}

Image * pgm_load(FILE * file)
{
    int version, width, height, max_value;
    fscanf(file, "P%d %d %d %d\n", &version, &width, &height, &max_value);

    if (version == 2)
    {
        Image_Format format = {GL_UNSIGNED_BYTE, GL_LUMINANCE, {width, height, 1}};
        Image * image = image_new(format);
        unsigned char * pixels = (unsigned char *) image->pixels;

        for (int i = 0; i != width * height; ++ i)
        {
            int value;
            fscanf(file, "%d", &value);

            * pixels ++ = value;
        }

        image_flip(image);

        fclose(file);

        return image;
    }
    else if (version == 5)
    {
        Image_Format format = {GL_UNSIGNED_BYTE, GL_LUMINANCE, {width, height, 1}};
        Image * image = image_new(format);

        unsigned char * pixels = (unsigned char *) image->pixels;

        for (int i = 0; i != width * height; ++ i)
        {
            int value = fgetc(file);
            * pixels ++ = value;
        }

        fclose(file);

        image_flip(image);

        return image;
    }

    return NULL;
}

Image * ppm_load(FILE * file)
{
    int version, width, height, max_value;
    fscanf(file, "P%d %d %d %d\n", &version, &width, &height, &max_value);

    if (version == 3)
    {
        Image_Format format = {GL_UNSIGNED_BYTE, GL_RGB, {width, height, 1}};
        Image * image = image_new(format);
        unsigned char * pixels = (unsigned char *) image->pixels;

        for (int i = 0; i != width * height; ++ i)
        {
            int red, green, blue;
            fscanf(file, "%d %d %d", &red, &green, &blue);

            * pixels ++ = red;
            * pixels ++ = green;
            * pixels ++ = blue;
        }

        image_flip(image);

        fclose(file);

        return image;
    }
    else if (version == 6)
    {
        Image_Format format = {GL_UNSIGNED_BYTE, GL_RGB, {width, height, 1}};
        Image * image = image_new(format);
        unsigned char * pixels = (unsigned char *) image->pixels;

        for (int i = 0; i != width * height * 3; ++ i)
        {
            int value = fgetc(file);
            * pixels ++ = value;
        }

        fclose(file);

        image_flip(image);

        return image;
    }

    return NULL;
}

