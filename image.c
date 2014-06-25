#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "file.h"
#include "file_image.h"
#include "image.h"
#include "memory.h"
#include "string.h"

#ifdef CYGWIN
static char const CYGPATH[] = "cygpath -w";
#endif

int image_type_to_size(GLenum const type)
{
    switch (type)
    {
        case GL_BITMAP:
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return 1;
    
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_HALF_FLOAT_ARB:
            return 2;

        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT:
            return 4;

        case GL_DOUBLE:
            return 8;
#ifdef GL_COMPLEX_CK
        case GL_COMPLEX_CK:
            return sizeof(complex);
#endif
    }

    return 0;
}

static int size_to_dimension(Size const size_)
{
    int const * size = (int const *) &size_;
    int i;

    for (i = 2; i >= 0; --i)
    {
        switch (size[i])
        {
            case 0:
                return 0;

            case 1:
                break;

            default:
                return i + 1;
        }
    }

    return i + 1;
}

int image_format_dimension(Image_Format format)
{
    return size_to_dimension(format.size);
}

static GLint get_modulo(GLint value)
{
    if (value % 8 == 0)
        return 8;

    if (value % 4 == 0)
        return 4;

    if (value % 2 == 0)
        return 2;

    return 1;
}

int image_format_pixels(Image_Format format)
{
    return size_total(format.size);
}

int image_format_pixel_size(Image_Format format)
{
    return image_type_to_size(format.type) * format_to_size(format.format);
}


Size image_format_total_size(Image_Format format)
{
    Size total_size;

    total_size.x = image_type_to_size(format.type) * format_to_size(format.format) * format.size.x;
    total_size.y = total_size.x * format.size.y;
    total_size.z = total_size.y * format.size.z;

    return total_size;
}

Size image_format_stride(Image_Format format)
{
    Size stride = size_stride(format.size);
    int pixel_size = image_format_pixel_size(format);

    stride.x *= pixel_size;
    stride.y *= pixel_size;
    stride.z *= pixel_size;

    return stride;
}

static GLint get_alignment(Image const * image)
{
    GLsizei const pixel_size = image_format_pixel_size(image->format);
    GLint const image_alignment = get_modulo((long) image->pixels);
    GLint const image_size_alignment = get_modulo(image->format.size.x * pixel_size);

    return image_alignment < image_size_alignment
        ? image_alignment
        : image_size_alignment;
}

GLenum size_to_format(GLsizei size)
{
    switch (size)
    {
        case 1: return GL_LUMINANCE;
        case 2: return GL_LUMINANCE_ALPHA;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
    }

    error_fail("unknown size to format");
    return GL_LUMINANCE;
}

GLsizei format_to_size(GLenum const format)
{
    switch (format)
    {
        case GL_COLOR_INDEX:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_LUMINANCE:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
            return 1;

        case GL_LUMINANCE_ALPHA:
            return 2;

        case GL_BGR:
        case GL_RGB:
            return 3;

        case GL_BGRA:
        case GL_RGBA:
            return 4;
    }

    return 0;
}

int image_format_bytes(Image_Format format)
{
    return
        size_total(format.size) *
        image_type_to_size(format.type) *
        format_to_size(format.format);
}

#if 0
int image_format_equal(Image_Format format_1, Image_Format format_2)
{
    return
        format_1.type   == format_2.type   &&
        format_1.format == format_2.format &&
        format_1.size.x == format_2.size.x &&
        format_1.size.y == format_2.size.y &&
        format_1.size.z == format_2.size.z;
}
#endif

static char const * type_to_string(GLenum type)
{
    switch (type)
    {
        case GL_BITMAP:         return "bitmap";
        case GL_BYTE:           return "byte";
        case GL_UNSIGNED_BYTE:  return "unsigned byte";
        case GL_SHORT:          return "short";
        case GL_UNSIGNED_SHORT: return "unsigned short";
        case GL_INT:            return "int";
        case GL_UNSIGNED_INT:   return "unsigned int";
        case GL_HALF_FLOAT_ARB: return "half";
        case GL_FLOAT:          return "float";
        case GL_DOUBLE:         return "double";
    }

    return NULL;
}

static char const * format_to_string(GLenum format)
{
    switch (format)
    {
        case GL_COLOR_INDEX:     return "color index";
        case GL_STENCIL_INDEX:   return "stencil index";
        case GL_DEPTH_COMPONENT: return "depth component";
        case GL_LUMINANCE:       return "luminance";
        case GL_LUMINANCE_ALPHA: return "luminance alpha";
        case GL_RED:             return "red";
        case GL_GREEN:           return "green";
        case GL_BLUE:            return "blue";
        case GL_ALPHA:           return "alpha";
        case GL_BGR:             return "bgr";
        case GL_BGRA:            return "bgra";
        case GL_RGB:             return "rgb";
        case GL_RGBA:            return "rgba";
    }

    return NULL;
}

void image_format_print(Image_Format format)
{
    printf("type = %s\n", type_to_string(format.type));
    printf("format = %s\n", format_to_string(format.format));
    printf("size = %dx%dx%d\n", format.size.x, format.size.y, format.size.z);
}

void image_destroy(Image * image)
{
    if (!image)
        return;

    free(image->pixels);
    free(image);
}

void image_print(Image const * image)
{
    Image_Format format = image->format;

    image_format_print(format);

    int count = format_to_size(format.format) * size_total(format.size);
    int i;

    switch (format.type)
    {
        case GL_FLOAT:
        {
            float * source = (float *) image->pixels;
            for (i = 0; i < count; ++ i) printf("%f ", source[i]);
            puts("");
        }
        break;

        default:
        // TODO all the rest
        break;
    }
}

void image_draw_layer(Image const * image, int layer)
{
    Image_Format format = image->format;

    // cleanup
    int pixel_size = image_format_pixel_size(image->format);
    int size_2d = image->format.size.x * format.size.y;
    unsigned char * pixels = (unsigned char *) image->pixels + size_2d * pixel_size * layer;

    glPixelStorei(GL_UNPACK_ALIGNMENT, get_alignment(image));
    glDrawPixels(format.size.x, format.size.y, format.format, format.type, pixels);
}

void image_draw(Image const * image)
{
    Image_Format format = image->format;

    glPixelStorei(GL_UNPACK_ALIGNMENT, get_alignment(image));
    glDrawPixels(format.size.x, format.size.y, format.format, format.type, image->pixels);
}

#if 0
void image_draw_region(Image const * image, Size min, Size max)
{
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image->format.size.x);
//    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, image->format.size.y);

    glPixelStorei(GL_UNPACK_SKIP_PIXELS, min.x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, min.y);
//    glPixelStorei(GL_UNPACK_SKIP_IMAGES, min.z);

    glDrawPixels(max.x - min.x, max.y - min.y, format.format, format.type, image->pixels);
}
#endif

Image * image_create(Image_Format format, void * pixels)
{
    Image * image = calloc_size(Image);
    image->format = format;
    image->pixels = pixels;
    image->palette = NULL;
    image->device  = NULL;

    return image;
}

Image * image_copy(Image const * image)
{
    Image_Format format = image->format;

    Image * new_image = image_new(format);
    memcpy(new_image->pixels, image->pixels, image_format_bytes(format));

    return new_image;
}

Image * image_new(Image_Format format)
{
    unsigned bytes = image_format_bytes(format);
    return image_create(format, calloc(1, bytes));
}

Image * image_grey_gradient(GLenum type)
{
    Image_Format format = {type, GL_LUMINANCE, {512, 512, 1}};
    Image * image = image_new(format);

    Size size = format.size;

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        {
            unsigned char * pixels = (unsigned char *) image->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int index = size_index(size, i, j, k);
                pixels[index] = (unsigned short) (255.0 * j / (size.y - 1));
            }
        }
        break;

        case GL_UNSIGNED_SHORT:
        {
            unsigned short * pixels = (unsigned short *) image->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int index = size_index(size, i, j, k);
                pixels[index] = (unsigned short) (65535.0 * j / (size.y - 1));
            }
        }
        break;

        case GL_FLOAT:
        {
            float * pixels = (float *) image->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int index = size_index(size, i, j, k);
                pixels[index] = (float) j / (size.y - 1);
            }
        }
        break;

        default:
            fprintf(stderr, "unsupported type for grey gradient\n");
            break;
    }


    return image;
}

Image * image_read(Viewport viewport, GLenum format, GLenum type)
{
    Image_Format image_format = {type, format, {viewport.width, viewport.height, 1}};
    Image * image = image_new(image_format);
    GLubyte * pixels = (GLubyte *) image->pixels;

    viewport_print(viewport); puts(""); // XXX debug
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // TODO optimize this
    glReadPixels(viewport.x, viewport.y, viewport.width, viewport.height, format, type, pixels);

    return image;
}

void image_store_unpack(Image const * image)
{
    Image_Format format = image->format;
    GLsizei dimension = size_to_dimension(format.size);
    GLint alignment = get_alignment(image);

    switch (dimension)
    {
        default:
            assert("invalid dimension" == NULL);
            exit(EXIT_FAILURE);

        case 3:
            glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, format.size.y);
            glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
            /* fall through */

        case 2:
            glPixelStorei(GL_UNPACK_ROW_LENGTH, format.size.x);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            /* fall through */

        case 1:
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
            glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
            glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
            break;
    }
}

void image_store_unpack_region(Image const image, GLint const position[3], Size const size)
{
    unsigned const dimension = size_to_dimension(size);
    Image_Format const image_format = image.format;
    Size const image_size = image_format.size;
    /* GLint const alignment = format_get_alignment(image_format); */

    switch (dimension)
    {
        default:
            assert("invalid dimension" == NULL);
            exit(EXIT_FAILURE);

        case 3:
            glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, image_size.y);
            glPixelStorei(GL_UNPACK_SKIP_IMAGES, position[2]);
            /* fall through */

        case 2:
            glPixelStorei(GL_UNPACK_ROW_LENGTH, image_size.x);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, position[1]);
            /* fall through */

        case 1:
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, position[0]);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); /* XXX optimize */
            glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
            glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
            break;
    }
}

void image_store_pack(Image const image, Size const size)
{
    GLsizei const dimension = size_to_dimension(size);
    Image_Format const image_format = image.format;
    Size const image_size = image_format.size;

    switch (dimension)
    {
        default:
            assert("invalid dimension" == NULL);
            exit(EXIT_FAILURE);

        case 3:
            glPixelStorei(GL_PACK_IMAGE_HEIGHT, image_size.y);
            glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
            /* fall through */

        case 2:
            glPixelStorei(GL_PACK_ROW_LENGTH, image_size.x);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            /* fall through */

        case 1:
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_PACK_ALIGNMENT, 1); /* XXX optimize */
            glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
            break;
    }
}

void image_store_pack_region(Image const image, GLint const position[3], Size const size)
{
    GLsizei const dimension = size_to_dimension(size);
    Image_Format const image_format = image.format;
    Size const image_size = image_format.size;

    switch (dimension)
    {
        default:
            assert("invalid dimension" == NULL);
            exit(EXIT_FAILURE);

        case 3:
            glPixelStorei(GL_PACK_IMAGE_HEIGHT, image_size.y);
            glPixelStorei(GL_PACK_SKIP_IMAGES, position[2]);
            /* fall through */

        case 2:
            glPixelStorei(GL_PACK_ROW_LENGTH, image_size.x);
            glPixelStorei(GL_PACK_SKIP_ROWS, position[1]);
            /* fall through */

        case 1:
            glPixelStorei(GL_PACK_SKIP_PIXELS, position[0]);
            glPixelStorei(GL_PACK_ALIGNMENT, 1); /* XXX optimize */
            glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
            break;
    }
}

#if 1
char const * guess_mime_type_2(char const name_[])
{
    char temp_name[L_tmpnam];
    file_temporary_name(temp_name);
    char command[256] = "file -bIL %s > %s";
    sprintf(command, name_, temp_name);

    int status = system(command);
    error_check_arg(status != EXIT_SUCCESS, "failed retrieval of mime type on \"%s\"", name_);

    char * mime_type = file_read_text(temp_name, NULL);

    if (remove(temp_name) != 0)
        perror("failed to delete");

    return mime_type; // memory leak
}
#endif

char const * file_guess_mime_type(char const name_[])
{
    char name[256];
    int length = strlen(name_);
    int i;

    assert(length < 256);
    for (i = 0; i != length; ++ i)
    {
        name[i] = tolower((unsigned char) name_[i]);
    }
    name[length] = '\0';

    if (strstr(name, ".pnm") == &name[length - 4])
    {
        return PNM_MIME;
    }
    if (strstr(name, ".pgm") == &name[length - 4])
    {
        return PGM_MIME;
    }
    else if (strstr(name, ".ppm") == &name[length - 4])
    {
        return PPM_MIME;
    }
#ifdef GIF
    else if (strstr(name, ".gif") == &name[length - 4])
    {
        return GIF_MIME;
    }
#endif
#ifdef PNG
    else if (strstr(name, ".png") == &name[length - 4])
    {
        return PNG_MIME;
    }
#endif
#ifdef JPEG
    else if (strstr(name, ".jpeg") == &name[length - 5])
    {
        return JPEG_MIME;
    }
    else if (strstr(name, ".jpg") == &name[length - 4])
    {
        return JPEG_MIME;
    }
    else if (strstr(name, ".mpo") == &name[length - 4])
    {
        return MPO_MIME;
    }
#endif
#ifdef TIFF
    else if (strstr(name, ".tiff") == &name[length - 5])
    {
        return TIFF_MIME;
    }
    else if (strstr(name, ".tif") == &name[length - 4])
    {
        return TIFF_MIME;
    }
#endif
#ifdef EXR
    else if (strstr(name, ".exr") == &name[length - 4])
    {
        return EXR_MIME;
    }
#endif
    else if (strstr(name, ".raw") == &name[length - 4])
    {
        return RAW_MIME;
    }

    return NULL;
}

static Image * image_open_mime(char const name[], char const mime_type[])
{
    FILE * file = fopen(name, "rb");

         if (streq(mime_type, PNM_MIME))  return pnm_load(file);
    else if (streq(mime_type, PGM_MIME))  return pgm_load(file);
    else if (streq(mime_type, PPM_MIME))  return ppm_load(file);
#ifdef PNG
    else if (streq(mime_type, PNG_MIME))  return png_load(file);
#endif
#ifdef JPEG
    else if (streq(mime_type, JPEG_MIME)) return jpeg_load(file);
    else if (streq(mime_type, MPO_MIME))
    {
        if (! file)
            return NULL;
        fclose(file);
        return mpo_load(name);
    }
#endif
#ifdef GIF
    else if (streq(mime_type, GIF_MIME))  return gif_load(file);
#endif
#ifdef TIFF
    else if (streq(mime_type, TIFF_MIME)) return tiff_load(file);
#endif
#ifdef EXR
    else if (streq(mime_type, EXR_MIME))
    {
        if (! file)
            return NULL;
        fclose(file);
        return exr_load(name);
    }
#endif
    else if (streq(mime_type, RAW_MIME))
    {
        Vector ratio;
        if (! file)
            return NULL;
        fclose(file);
        return raw_load(name, &ratio);
    }

    return NULL;
}

Image * image_open(char const name[])
{

    char const * mime_type = file_guess_mime_type(name);
    if (mime_type)
        return image_open_mime(name, mime_type);

    char command[256];
    char temp_name[L_tmpnam];
    file_temporary_name(temp_name);
#ifdef CYGWIN
    sprintf(command, "convert `%s %s` png:`%s %s`", CYGPATH, name, CYGPATH, temp_name);
#else
    sprintf(command, "convert %s png:%s", name, temp_name);
#endif

    printf("executing \"%s\"\n", command);
    int status = system(command);
    error_check_arg(status != EXIT_SUCCESS, "unrecognized image format \"%s\"", name);

#ifdef PNG
    Image * image = image_open_mime(temp_name, PNG_MIME);

    if (remove(temp_name) != 0)
        perror("failed to delete");

    return image;
#else
    return NULL;
#endif
}

void image_clear(Image * image)
{
    int bytes = image_format_bytes(image->format);
    memset(image->pixels, 0, bytes);
}

void image_flip(Image * image)
{
    Image_Format format = image->format;
    Size size = format.size;
    int width  = size.x;
    int height = size.y;
    int depth = size.z;
    int row_size = width * image_format_pixel_size(format);

    unsigned char * buffer_1 = (unsigned char *) malloc(row_size);
    unsigned char * buffer_2 = (unsigned char *) malloc(row_size);

    for (int k = 0; k != depth; ++ k)
    {
        unsigned char * pixels = &((unsigned char *) image->pixels)[k * row_size * height];

        for (int i = 0; i != height / 2; ++ i)
        {
            int j = height - 1 - i;

            memcpy(buffer_1, &pixels[i * row_size], row_size);
            memcpy(buffer_2, &pixels[j * row_size], row_size);

            memcpy(&pixels[i * row_size], buffer_2, row_size);
            memcpy(&pixels[j * row_size], buffer_1, row_size);
        }
    }

    free(buffer_1);
    free(buffer_2);
}

int image_format_equal(Image_Format format_1, Image_Format format_2)
{
    if (format_1.format != format_2.format)
        return 0;

    if (format_1.type != format_2.type)
        return 0;

    return ! memcmp(& format_1.size, & format_2.size, sizeof(Size));
}

Image * image_stack(Image const * images[], int count)
{
    assert(count <= 0);

    if (count == 1)
        return image_copy(images[0]);

    Image_Format first_format = images[0]->format;

    for (int i = 1; i != count; ++ i)
    {
        Image_Format format = images[i]->format;
        error_check(! image_format_equal(first_format, format), "format must be same");
    }

    int image_size = image_format_bytes(first_format);

    Image_Format target_format = first_format;
    target_format.size.z = count;

    Image * image = image_new(target_format);

    for (int i = 0; i != count; ++ i)
    {
        unsigned char const * source = (unsigned char const *) images[i]->pixels;
        unsigned char * target = & ((unsigned char *) image->pixels)[i * image_size];

        memcpy(target, source, image_size);
    }

    return image;
}

Property * image_properties(char const name[], unsigned * count)
{
    char command[512];
    char output[L_tmpnam];
    file_temporary_name(output);

    sprintf(command, "exiftool -S %s > %s", name, output);
    system(command);

    unsigned line_count;
    char ** lines = file_read_text_lines(output, &line_count);

    Property * properties = malloc_array(Property, line_count);

    for (int i = 0; i != (int) line_count; ++ i)
    {
        char * line = lines[i];
        char * colon = strchr(line, ':');

        properties[i].key = strndup_(line, colon - line);
        properties[i].value = strdup(colon + 2);
    }

    * count = line_count;
    return properties;
}

