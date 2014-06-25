#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "math_.h"
#include "file_image.h"
#include "string.h"

char const * RAW_MIME = "image/x-raw";

static GLenum bits_to_type(int bits)
{
    switch (bits)
    {
        case 8: return GL_UNSIGNED_BYTE;
        case 16: return GL_UNSIGNED_SHORT;
        case 32: return GL_UNSIGNED_INT; // XXX or GL_UNSIGNED_FLOAT
    }

    return GL_UNSIGNED_BYTE;
}

Image * raw_load(char const name[], Vector * ratio)
{
    Image_Format format;
    int bits;

    char const * base = basename_(name);
    char const * const values = strchr(base, '-');
    error_check_arg(values == NULL, "bad filename \"%s\"", name);

    unsigned int result = sscanf(values, "-%d-%dx%dx%d-%fx%fx%f.raw", &bits, &format.size.x, &format.size.y, &format.size.z, &ratio->x, &ratio->y, &ratio->z);

    if (result == 7)
    {
        assert(bits == 8 || bits == 16 || bits == 32);
        format.type = bits_to_type(bits);
    }
    else
    {
        result = sscanf(values, "-%dx%dx%d-%fx%fx%f.raw", &format.size.x, &format.size.y, &format.size.z, &ratio->x, &ratio->y, &ratio->z);

        format.type = GL_UNSIGNED_BYTE;

        if (result != 6)
        {
            result = sscanf(values, "-%dx%dx%d.raw", &format.size.x, &format.size.y, &format.size.z);
            assert(result == 3);
            error_check_arg(result != 3, "bad size in file name %s", name);

            ratio->x =
            ratio->y =
            ratio->z = 1;
        }
    }

    if (! power_of_two(format.size.x) ||
        ! power_of_two(format.size.y) ||
        ! power_of_two(format.size.z))
    {
#if 0
        // XXX linking error
        error_check(! GL_ARB_texture_non_power_of_two,
             "unsupported non power of two textures");
#endif
    }

    format.format = GL_LUMINANCE;

    FILE * const file = fopen(name, "rb");
    error_check_arg(! file, "failed to open \"%s\"", name);

    Image * image = image_new(format);

    unsigned const byte_count = image_format_bytes(format);
    fread(image->pixels, 1, byte_count, file);
    fclose(file);

    return image;
}

void raw_save(Image const * image, FILE * file)
{
    unsigned const byte_count = image_format_bytes(image->format);
    fwrite(image->pixels, 1, byte_count, file);
    fclose(file);
}

