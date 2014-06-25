#include <assert.h>
#include <float.h>
#include <stdio.h>

#include "error.h"
#include "half.h"
#include "image_process.h"
#include "kernel.h"
#include "math_.h"
#include "memory.h"
//#include "perlin.h"
#include "print.h"
#include "size.h"
#include "utils.h"

#if 1
#define extract(source, target, source_components, target_components, source_index, target_index, count) \
{\
    int i; \
    for (i = 0; i != count; ++ i) \
    { \
        target[i * target_components + target_index] = \
        source[i * source_components + source_index]; \
    } \
}
#else
#define extract(source, target, source_components, target_components, swizzle, count) \
{\
    int i, j; \
    for (i = 0; i != count; ++ i) \
    { \
        for (j = 0; j != target_components; ++ j) \
        { \
            int index = swizzle[j]; \
            target[i * target_components + j] = (index == -1) ? 0 : \
            source[i * source_components + index]; \
        } \
    } \
}
#endif

static int extraction_index(GLenum format, GLenum component)
{
    switch (component)
    {
        case GL_LUMINANCE:
            if (format == GL_LUMINANCE_ALPHA || format == GL_LUMINANCE)
                return 0;
            break;

        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
            switch (format)
            {
                case GL_RGB:
                case GL_RGBA:
                    return component - GL_RED;

                case GL_BGR:
                case GL_BGRA:
                    return format_to_size(format) - 1 - (component - GL_RED);
            }
            break;

        case GL_ALPHA:
            switch (format)
            {
                case GL_LUMINANCE_ALPHA:
                    return 1;
                
                case GL_RGBA:
                case GL_BGRA:
                    return 3;
            }
            break;
    }
    
    return -1;
}

Image * image_retype(Image const * image, GLenum target_type)
{
    Image_Format format = image->format;
    
    error_check(format.type == target_type, "images must have different type");
    
    Image_Format new_format = format;
    new_format.type = target_type;
    Image * new_image = image_new(new_format);
    
    int count = format_to_size(format.format) * size_total(format.size);
    int i;

    // TODO: convert to double as intermediate
    
    switch (format.type)
    {
        case GL_UNSIGNED_BYTE:
        {
            unsigned char const * source = (unsigned char const *) image->pixels;
            switch (target_type)
            {
                case GL_UNSIGNED_SHORT:
                {
                    unsigned short * target = (unsigned short *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] << 8;
                }
                break;
                    
                case GL_FLOAT:
                {
                    float * target = (float *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] / 255.0;
                }
                break;
                    
                case GL_DOUBLE:
                {
                    double * target = (double *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] / 255.0;
                }
                break;
            }
        }
        break;
            
        case GL_UNSIGNED_SHORT:
        {
            unsigned short const * source = (unsigned short const *) image->pixels;
            switch (target_type)
            {
                case GL_UNSIGNED_BYTE:
                {
                    unsigned char * target = (unsigned char *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] >> 8;
                }
                break;
                    
                case GL_FLOAT:
                {
                    float * target = (float *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] / 65535.0;
                }
                break;
                    
                case GL_DOUBLE:
                {
                    double * target = (double *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] / 65535.0;
                }
                break;
            }
        }
        break;

        case GL_HALF_FLOAT_ARB:
        {
            unsigned short const * source = (unsigned short const *) image->pixels;
            switch (target_type)
            {
                case GL_UNSIGNED_BYTE:
                {
                    unsigned char * target = (unsigned char *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = clamp(half_to_float(source[i])) * 255.0;
                }
                break;
                    
                case GL_UNSIGNED_SHORT:
                {
                    unsigned short * target = (unsigned short *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = clamp(half_to_float(source[i])) * 65535.0;
                }
                break;
                    
                case GL_FLOAT:
                {
                    float * target = (float *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = half_to_float(source[i]);
                }
                break;
                    
                case GL_DOUBLE:
                {
                    double * target = (double *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = half_to_float(source[i]);
                }
                break;
            }
        }
        break;
            
        case GL_FLOAT:
        {
            float const * source = (float const *) image->pixels;
            switch (target_type)
            {
                case GL_UNSIGNED_BYTE:
                {
                    unsigned char * target = (unsigned char *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = clamp(source[i]) * 255.0;
                }
                    break;
                    
                case GL_UNSIGNED_SHORT:
                {
                    unsigned short * target = (unsigned short *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = clamp(source[i]) * 65535.0;
                }
                    break;
                    
                case GL_HALF_FLOAT_ARB:
                {
                    unsigned short * target = (unsigned short *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = half_from_float(source[i]);
                }
                    break;
                    
                case GL_DOUBLE:
                {
                    double * target = (double *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i];
                }
                    break;
            }
        }
        break;
            
        case GL_DOUBLE:
        {
            double const * source = (double const *) image->pixels;
            switch (target_type)
            {
                case GL_UNSIGNED_BYTE:
                {
                    unsigned char * target = (unsigned char *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] * 255.0; // TODO dclamp
                }
                    break;
                    
                case GL_UNSIGNED_SHORT:
                {
                    unsigned short * target = (unsigned short *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i] * 65535.0;
                }
                    break;
                    
                case GL_HALF_FLOAT_ARB:
                {
                    unsigned short * target = (unsigned short *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = half_from_float(source[i]);
                }
                    break;
                    
                case GL_FLOAT:
                {
                    float * target = (float *) new_image->pixels;
                    for (i = 0; i < count; ++ i) target[i] = source[i];
                }
                    break;
            }
        }
            break;
    }
    
    return new_image;
}

Image * image_reformat(Image const * image)
{
    Image * image2 = image_retype(image, GL_FLOAT);

    if (image2->format.format == GL_RGBA)
        return image2;

    Image_Format format = image->format;
    format.type = GL_FLOAT;
    format.format = GL_RGBA;

    Image * image3 = image_new(format);
    image_add_alpha(image2, image3);

    return image3;
}

// YUV
#define color_to_luminance(c) (0.299 * (c).r + 0.587 * (c).g + 0.114 * (c).b)

Image * image_rgb_to_gray(Image const * source)
{
    Image_Format source_format = source->format;

    error_check(source_format.format != GL_RGB && source_format.format != GL_RGBA, "format must be RGB or RGBA");
    error_check(source_format.type != GL_FLOAT, "type must be float");

    int pixel_count = image_format_pixels(source_format);

    Image_Format target_format = source_format;
    target_format.format = source_format.format == GL_RGB ? GL_LUMINANCE : GL_LUMINANCE_ALPHA;

    Image * target        = image_new(target_format);
    float * target_pixels = target->pixels;

    switch (source_format.format)
    {
        case GL_RGB:
            {
                Color const * source_pixels = source->pixels;

                for (int i = 0; i != pixel_count; ++ i)
                {
                    Color c = source_pixels[i];
                    target_pixels[i] = color_to_luminance(c);
                }

            }
            break;

        case GL_RGBA:
            {
                Color4 const * source_pixels = source->pixels;

                for (int i = 0; i != pixel_count; ++ i)
                {
                    Color4 c = source_pixels[i];
                    target_pixels[2 * i + 0] = color_to_luminance(c.c);
                    target_pixels[2 * i + 1] = c.a;
                }
            }
            break;
    }

    return target;
}

Image * image_extract(Image const * image, GLenum target_format, GLenum source_component, GLenum target_component)
{
    Image_Format format = image->format;
    
    int source_components = format_to_size(format.format);
    int target_components = format_to_size(target_format);
    int source_index = extraction_index(format.format, source_component);
    int target_index = extraction_index(target_format, target_component);
    int count = size_total(format.size);
    
    error_check(source_index == -1, "illegal source component");
    error_check(target_index == -1, "illegal target component");
    
    Image_Format new_format = format;
    new_format.format = target_format;
    Image * new_image = image_new(new_format);
    image_clear(new_image);
    
    //process(format.type, image->pixels, new_image->pixels, source_components, target_components, swizzle, count);
    
    switch (image_type_to_size(format.type))
    {
        case 1:
        {
            unsigned char const * source = (unsigned char const *) image->pixels;
            unsigned char * target = (unsigned char *) new_image->pixels;
            extract(source, target, source_components, target_components, source_index, target_index, count);
        }
            break;
            
        case 2:
        {
            short const * source = (short const *) image->pixels;
            short * target = (short *) new_image->pixels;
            extract(source, target, source_components, target_components, source_index, target_index, count);
        }
            break;
            
        case 4:
        {
            float const * source = (float const *) image->pixels;
            float * target = (float *) new_image->pixels;
            extract(source, target, source_components, target_components, source_index, target_index, count);
        }
            break;
            
        case 8:
        {
            double const * source = (double const *) image->pixels;
            double * target = (double *) new_image->pixels;
            extract(source, target, source_components, target_components, source_index, target_index, count);
        }
            break;
            
        default:
            error_check(1, "bad pixel type");
    }
    
    return new_image;
}

#if 0
static void process(GLenum type, void const * source, void * target, int source_components,
 int target_components, int const swizzle[4], int count)
{
    switch (type_to_size(type))
    {
        case 1:
            {
                unsigned char const * source = (unsigned char const *) image->pixels;
                unsigned char * target = (unsigned char *) new_image->pixels;
                extract(source, target, source_components, target_components, swizzle, coun
t);
            }
            break;

        case 2:
            {
                short const * source = (short const *) image->pixels;
                short * target = (short *) new_image->pixels;
                extract(source, target, source_components, target_components, swizzle, coun
t);
            }
            break;

        case 4:
            {
                float const * source = (float const *) image->pixels;
                float * target = (float *) new_image->pixels;
                extract(source, target, source_components, target_components, swizzle, coun
t);
            }
            break;

        default:
            error_check(1, "bad pixel type");
    }
}
#endif

#if 0
Image * image_pack(Image * red, Image * green, Image * blue, Image * alpha)
{
    error_check(! red, "red image must be specified");
    Image_Format format = red->format;
    Image * new_image = image_new(format);

    if (green)
    {        error_check(! blue, "blue image must be specified");
        format.format = alpha ? GL_RGBA : GL_RGB;
        static int const swizzle1[4] = {0, -1, -1, -1};        process(format.type, red->pixels, new_image->pixels, source_components, target_comp
onents, swizzle1, count);
        static int const swizzle2[4] = {-1, 0, -1, -1};        process(format.type, green->pixels, new_image->pixels, source_components, target_co
mponents, swizzle2, count);
        static int const swizzle3[4] = {-1, -1, 0, -1};
        process(format.type, blue->pixels, new_image->pixels, source_components, target_com
ponents, swizzle3, count);        if (alpha)
        {
            static int const swizzle4[4] = {-1, -1, -1, 0};
            process(format.type, alpha->pixels, new_image->pixels, source_components, targe
t_components, swizzle3, count);
        }
    }
    else
    {
        error_check(blue, "blue image must not be specified");
        error_check(! alpha, "alpha image must be specified");
        format.format = GL_LUMINANCE_ALPHA;
        static int const swizzle1[4] = {0, -1, -1, -1};        process(format.type, red->pixels, new_image->pixels, source_components, target_comp
onents, swizzle1, count);
        static int const swizzle2[4] = {-1, 0, -1, -1};
        process(format.type, alpha->pixels, new_image->pixels, source_components, target_co
mponents, swizzle2, count);
    }
}
#endif

void image_convert(Image * image, Image_Format target_format)
{
    // TODO all other formats
    // assume ubyte to float

    int const pixel_count = size_total(image->format.size);
    unsigned char * source = (unsigned char *) image->pixels;
    float * target = malloc_array(float, pixel_count);

    for (int i = 0; i != pixel_count; ++ i)
    {
        target[i] = source[i] / 255.0;
    }

    image->pixels = target;
    free(source);

    image->format.type = target_format.type; // XXX replace all
}

void image_add_alpha(Image const * source, Image * target)
{
    error_check(source->format.type != GL_FLOAT, "source type must be float");
    error_check(target->format.type != GL_FLOAT, "target type must be float");
    error_check(source->format.format != GL_RGB, "source format must be rgb");
    error_check(target->format.format != GL_RGBA, "target format must be rgba");
    error_check(!size_equal(source->format.size, target->format.size), "source and target size must be equal");

    float const * source_pixels = (float const *) source->pixels;
    float * target_pixels = (float *) target->pixels;
    Size size = source->format.size;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        * target_pixels ++ = * source_pixels ++;
        * target_pixels ++ = * source_pixels ++;
        * target_pixels ++ = * source_pixels ++;
        * target_pixels ++ = 1;
    }
}

void image_remove_alpha(Image const * source, Image * target)
{
    error_check(source->format.type != GL_FLOAT, "source type must be float");
    error_check(target->format.type != GL_FLOAT, "target type must be float");
    error_check(!(source->format.format == GL_RGBA && target->format.format == GL_RGB) && !(source->format.format == GL_LUMINANCE_ALPHA && target->format.format == GL_LUMINANCE), "source format must be rgba and target must must be rgb or source must be luminance alpha and target must be luminance");
//    error_check(target->format.format != GL_RGB, "target format must be rgb");
    error_check(!size_equal(source->format.size, target->format.size), "source and target size must be equal");

    float const * source_pixels = (float const *) source->pixels;
    float * target_pixels = (float *) target->pixels;
    Size size = source->format.size;

    if (source->format.format == GL_RGBA)
    {
        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            * target_pixels ++ = * source_pixels ++;
            * target_pixels ++ = * source_pixels ++;
            * target_pixels ++ = * source_pixels ++;
            source_pixels ++;
        }
    }
    else if (source->format.format == GL_LUMINANCE_ALPHA)
    {
        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            * target_pixels ++ = * source_pixels ++;
            source_pixels ++;
        }
    }
}

int pick_lowest(float a, float b, float c)
{
    if (a < c)
        return a < b ? -1 : 0;

    return c < b ? +1 : 0;
}

#if 0

void image_resize(Image const * source_image, Image * target_image)
{
    Color const * source = (Color const *) source_image->pixels;
    //Color * target = (Color *) target_image->pixels;

    Size size = source_image->format.size;

#if 0
    Size d = {1, size.x, size.x * size.y};

    float * energies = calloc_array(float, size.x);

    for (int i = 0; i != size.y; ++ i)
    for (int j = 0; j != size.x; ++ j)
    {
        int index = size_index(size, 0, i, j);
        Color color = source[index];
        Color color_left  = j == 0 ? color : source[index - d.x];
        Color color_right = j == size.x - 1 ? color : source[index + d.x];

        float dx = fmax(color_distance(color, color_left), color_distance(color, color_right));
        float dy = 0; // TODO
        energies[index] = dx + dy;
    }

    // calculate paths

    int * paths = malloc_array(int, size.x * size.y);

    for (int j = 0; j != size.x; ++ j)
    {
        paths[j] = j;
    }

    for (int i = 1; i != size.y; ++ i)
    for (int j = 0; j != size.x; ++ j)
    {
        int index = size_index(size, 0, i, j);
        int index_left = j == 0 ? index : index - d.x;
        int index_right = j == size.x - 1 ? index : index + d.x;

        // TODO could use a window for scanning
        int delta = pick_lowest(energies[index_left], energies[index], energies[index_right]);
        paths[index] = paths[index - d.y] + delta;
    }

    // pick minimum 
    float min_energy = FLT_MAX;
    int min_index = -1;

    for (int j = 0; j != size.x; ++ j)
    {
        float energy = total[j];
        if (energy > min_energy)
            continue;

        min_energy = energy;
        min_index = j;
    }
#endif

    int delta_x = target_size.x - size.x;

    int * seam_indices = malloc_array(int, size.x);
    for (int i = 0; i != size.x; ++ i)
    {
        seam_indices[i] = i;
    }

    // create new image

    Size target_size = target->format.size;

    for (int i = 0; i != size.y; ++ i)
    {
        int source_index = size_index(size, 0, i, 0);
        int target_index = size_index(target_size, 0, i, 0);
        //int seam_index = i == 0 ? min_index : paths[];

        for (j = 0; j != -delta_x; ++ j)
        {
            memcpy(&target[target_index], &source[source_index], seam_index * sizeof(Color));
            source_index += seam_index + 1;
            target_index += width;
        }

        memcpy(&target[target_index + seam_index], &source[source_index + seam_index + 1], (size.x - seam_index - 1) * sizeof(Color));
    }

    free(seam_indices);
    //free(total);
    //free(paths);
    //free(energies);
}
#endif

void image_fill(Image * image, Texture texture, void * texture_data, Box box)
{
    error_check(image->format.type != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_LUMINANCE, "bad format");

    Size size = image->format.size;
    Vector v = ORIGIN;

    float * target = (float *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    {
        float tz = (float) i / size.z;
        v.z = interpolate(box.min.z, box.max.z, tz);

        for (int j = 0; j != size.y; ++ j)
        {
            float ty = (float) j / size.y;
            v.y = interpolate(box.min.y, box.max.y, ty);

            for (int k = 0; k != size.x; ++ k)
            {
                float tx = (float) k / size.x;
                v.x = interpolate(box.min.x, box.max.x, tx);

                Color color = texture(texture_data, v);
                * target ++ = color.r;
            }
        }
    }
}

void image_fill_color(Image * image, Texture texture, void * texture_data, Box box)
{
    error_check(image->format.type != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_RGBA, "bad format");

    Size size = image->format.size;
    Vector v = ORIGIN;

    Color * target = (Color *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    {
        float tz = (float) i / size.z;
        v.z = interpolate(box.min.z, box.max.z, tz);

        for (int j = 0; j != size.y; ++ j)
        {
            float ty = (float) j / size.y;
            v.y = interpolate(box.min.y, box.max.y, ty);

            for (int k = 0; k != size.x; ++ k)
            {
                float tx = (float) k / size.x;
                v.x = interpolate(box.min.x, box.max.x, tx);

                Color color = texture(texture_data, v);
                * target ++ = color;
            }
        }
    }
}

#if 1
static Color gradient_texture(void * data, Vector position)
{
    Color const * values_ = (Color const *) data;
    float u = position.x;
    float v = position.y;

    Color const values[2][2] =
    {
        {values_[0], values_[1]},
        {values_[2], values_[3]}
    };

    return color_interpolate_bilinear(values, u, v);
}

void image_gradient(Image * image, Color const values[2][2])
{
    image_fill(image, gradient_texture, (void *) values, ORIGIN_BOX);
}
#endif

float image_sample_1D(Image const * image, float position)
{
    error_check(image->format.type != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_LUMINANCE, "bad format");

    Size size = image->format.size;

    float x = position * size.x;

    int j = floor(x);

    if (j < 0 || j >= size.x)
        return 0; // -1;

    float u = x - j;
    int index = j;
    float const * pixels = (float *) image->pixels;

    return interpolate(pixels[index], pixels[index + 1], u);
}

float image_sample_2D(Image const * image, Vector position)
{
    error_check(image->format.type != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_LUMINANCE, "bad format");

    Size size = image->format.size;
    Size d = size_stride(size);

    float x = position.x * size.x;
    float y = position.y * size.y;

    int j = floor(x);
    int i = floor(y);

    if (i < 0 || i >= size.y - 1  || j < 0 || j >= size.x - 1)
        return -1;

    float u = x - j;
    float v = y - i;

    int index = size_index(size, 0, i, j);

    float const * pixels = (float *) image->pixels;
    float const values[2][2] =
    {
        {pixels[index], pixels[index + d.x]},
        {pixels[index + d.y], pixels[index + d.x + d.y]}
    };

    return interpolate_bilinear(values, u, v);
}

int image_sample_position(Image const * image, int j, int i, Size size, Size d, Border border, int samples[4])
{
    int dj, di;

    switch (border)
    {
        case BORDER_BLACK:
            return 0;

        case BORDER_CLAMP:
            if (j < 0) j = 0;
            if (i < 0) i = 0;

            if (j >= size.x - 1) {j = size.x - 1; dj = 0;}
            else dj = d.x;

            if (i >= size.y - 1) {i = size.y - 1; di = 0;}
            else di = d.y;

            break;

        default:
        case BORDER_MIRROR: // unsupported
        case BORDER_WRAP:
            while (j >= size.x) j -= size.x;
            while (i >= size.y) i -= size.y;
            while (j < 0) j += size.x;
            while (i < 0) i += size.y;

            assert(j >= 0 && j < size.x);
            assert(i >= 0 && i < size.y);

            dj = (j == size.x - 1) ? -j : d.x;
            di = (i == size.y - 1) ? (-i * d.y) : d.y;

            break;
    }

    int index = size_index(size, 0, i, j);

    samples[0] = index;
    samples[1] = index      + dj;
    samples[2] = index + di;
    samples[3] = index + di + dj;

    return 1;
}

Color4 image_sample_color4_2D(Image const * image, Vector position, Border border)
{
    error_check(image->format.type   != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_RGBA,  "bad format");

    Size size = image->format.size;
    Size d = size_stride(size);

    float x = position.x * size.x;
    float y = position.y * size.y;

    int j = floor(x);
    int i = floor(y);

    float u = x - j;
    float v = y - i;

    int samples[4];
    if (! image_sample_position(image, j, i, size, d, border, samples))
    {
        Color4 black = {BLACK, 1};
        return black;
    }

    Color4 const * pixels = (Color4*) image->pixels;
    Color4 const values[2][2] =
    {
        {pixels[samples[0]], pixels[samples[1]]},
        {pixels[samples[2]], pixels[samples[3]]}
    };

    return color4_interpolate_bilinear(values, u, v);
}

#if 1
static int index_of_sample(Image const * image, Vector position, Border border)
{
    Size size = image->format.size;
    Size d = size_stride(size);

    float x = position.x;
    float y = position.y;
    float z = position.z;
//    float x = position.x * size.x;
//    float y = position.y * size.y;

    int j = floor(x);
    int i = floor(y);
    int k = floor(z);

//    float u = x - j;
//    float v = y - i;

    int dj, di;

    switch (border)
    {
        case BORDER_BLACK: //return -1;
            if (i < 0 || i >= size.y  || j < 0 || j >= size.x)
                return -1;
            break;

        case BORDER_CLAMP:
            if (j < 0) j = 0;
            if (i < 0) i = 0;

            if (j >= size.x - 1) {j = size.x - 1; dj = 0;}
            else dj = d.x;

            if (i >= size.y - 1) {i = size.y - 1; di = 0;}
            else di = d.y;

            break;

        default:
        case BORDER_MIRROR: // unsupported
        case BORDER_WRAP:
            while (j >= size.x) j -= size.x;
            while (i >= size.y) i -= size.y;
            while (j < 0) j += size.x;
            while (i < 0) i += size.y;

            assert(j >= 0 && j < size.x);
            assert(i >= 0 && i < size.y);

            dj = (j == size.x - 1) ? -j : d.x;
            di = (i == size.y - 1) ? (-i * d.y) : d.y;

            break;
    }

    return size_index(size, k, i, j);
}

Color color_from_luminance(float luminance)
{
    Color c = {luminance, luminance, luminance};
    return c;
}

Color image_sample(Image const * image, Vector position, Border border)
{
    error_check(image->format.type   != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_RGB && image->format.format != GL_RGBA && image->format.format != GL_LUMINANCE,  "bad format");

    int index = index_of_sample(image, position, border);
    if (index == -1)
        return MAGENTA;

    if (image->format.format == GL_RGB)
        return ((Color const *) image->pixels)[index];

    if (image->format.format == GL_LUMINANCE)
        return color_from_luminance(((float const *) image->pixels)[index]);

    Color4 color4 = ((Color4 const *) image->pixels)[index];
    return color4.c;
}

unsigned short image_sample_index(Image const * image, Vector position, Border border)
{
    error_check(image->format.type   != GL_UNSIGNED_SHORT, "bad type");
    error_check(image->format.format != GL_LUMINANCE,  "bad format");

    int index = index_of_sample(image, position, border);
    if (index == -1)
        return -1;

    return (((unsigned short const *) image->pixels)[index]);
}
#endif

// XXX use image_sample_position
Color image_sample_color_2D(Image const * image, Vector position, Border border)
{
    error_check(image->format.type   != GL_FLOAT, "bad type");
    error_check(image->format.format != GL_RGB,  "bad format");

    Size size = image->format.size;
    Size d = size_stride(size);

    float x = position.x * size.x;
    float y = position.y * size.y;

    int j = floor(x);
    int i = floor(y);

    float u = x - j;
    float v = y - i;

    int dj, di;

    switch (border)
    {
        case BORDER_BLACK: return BLACK;
            if (i < 0 || i >= size.y  || j < 0 || j >= size.x)
                return MAGENTA;
            break;

        case BORDER_CLAMP:
            if (j < 0) j = 0;
            if (i < 0) i = 0;

            if (j >= size.x - 1) {j = size.x - 1; dj = 0;}
            else dj = d.x;

            if (i >= size.y - 1) {i = size.y - 1; di = 0;}
            else di = d.y;

            break;

        default:
        case BORDER_MIRROR: // unsupported
        case BORDER_WRAP:
            while (j >= size.x) j -= size.x;
            while (i >= size.y) i -= size.y;
            while (j < 0) j += size.x;
            while (i < 0) i += size.y;

            assert(j >= 0 && j < size.x);
            assert(i >= 0 && i < size.y);

            dj = (j == size.x - 1) ? -j : d.x;
            di = (i == size.y - 1) ? (-i * d.y) : d.y;

            break;
    }

    int index = size_index(size, 0, i, j);

    Color const * pixels = (Color *) image->pixels;
    Color const values[2][2] =
    {
        {pixels[index], pixels[index + dj]},
        {pixels[index + di], pixels[index + dj + di]}
    };

    return color_interpolate_bilinear(values, u, v);
}

static int choose_bin_count(Image_Format format)
{
    switch (format.type)
    {
        default:
        case GL_UNSIGNED_BYTE:  return 256;
        case GL_UNSIGNED_SHORT: return 65536;
    }
}

Image * image_histogram(Image const * image, int bin_count)
{
    Image_Format format = image->format;
    int pixel_count = size_volume(format.size);
    int channel_count = format_to_size(format.format);

    if (! bin_count)
        bin_count = choose_bin_count(format);

    Image_Format histogram_format = {GL_FLOAT, image->format.format, {bin_count, 1, 1}};
    Image * histogram = image_new(histogram_format);
    float * target = (float *) histogram->pixels;

    switch (format.type)
    {
        default:
            error_check(1, "unsupported type for histogram");
            break;

        case GL_UNSIGNED_BYTE:
        {
            unsigned char const * source = (unsigned char const *) image->pixels;
            for (int i = 0; i !=   pixel_count; ++ i)
            for (int j = 0; j != channel_count; ++ j)
            {
                int index = source[i * channel_count + j];
                target[index] += 1.0;
            }
        }
        break;

        case GL_UNSIGNED_SHORT:
        {
            unsigned short const * source = (unsigned short const *) image->pixels;
            for (int i = 0; i !=   pixel_count; ++ i)
            for (int j = 0; j != channel_count; ++ j)
            {
                int index = source[i * channel_count + j];
                target[index * channel_count + j] += 1.0;
            }
        }
        break;

        case GL_FLOAT:
        {
            float const * source = (float const *) image->pixels;
            for (int i = 0; i !=   pixel_count; ++ i)
            for (int j = 0; j != channel_count; ++ j)
            {
                float value = source[i * channel_count + j];
                if (isnan(value) || isinf(value) || value < 0)
                    continue;

                int index = floor(value * (bin_count - 1));
                target[index * channel_count + j] += 1.0;
            }
        }
    }

    return histogram;
}

void image_min_max(Image const * image, float * min, float * max)
{
    float const * source = (float const *) image->pixels; 
    Size size = image->format.size;

    * min = +FLT_MAX;
    * max = -FLT_MAX;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
         int index = size_index(size, i, j, k);
         float value = source[index];

         * min = fmin(* min, value);
         * max = fmax(* max, value);
    }
}

#if 0
typedef struct
{
    Image const * source;
    Moebius_Transform transform;
}
Moebius_Texture;

static Color moebius_texture(void * data, Vector position)
{
    Moebius_Texture const * texture_data = (Moebius_Texture const *) data;

    Complex z = {position.x, position.y};
    z = moebius_transform(z, &texture_data->transform);

    position.x = z.r;
    position.y = z.i;

    return image_sample_color_2D(texture_data->source, position, BORDER_WRAP);
}

void image_moebius_transform(Image const * source, Moebius_Transform transform, Image * target)
{
    Moebius_Texture texture_data = {source, transform};
    image_fill_color(target, moebius_texture, &texture_data, ORIGIN_BOX);
}

void complex_plot(Image * image, Complex_Function f, void * data, Box box)
{
    error_check(image->format.format != GL_RGBA, "format must be RGBA");

    Size size = image->format.size;
    Color * colors = (Color *) image->pixels;

    for (int i = 0; i != size.y; ++ i)
    {
        float y = (float) i * 2 / size.y - 1;

        for (int j = 0; j != size.x; ++ j)
        {
            float x = (float) j * 2 / size.x - 1;
            Complex z1 = {x, y};
            Complex z2 = f(data, z1);

            * colors ++ = color_from_complex(z2);
        }
    }
}
#endif

Image * image_convolve(Image const * source, float const weights[], int size, Border border)
{
    error_check(source->format.format != GL_RGB, "image format must be RGB");

    Image * target = image_new(source->format);

    Color const * source_pixels = (Color const *) source->pixels;
    Color       * target_pixels = (Color       *) target->pixels;

    int width  = source->format.size.x;
    int height = source->format.size.x;
    int size_2 = size / 2;

    int k;
    Color accumulator;

    for (int i = 0; i != height; ++ i)
    {
        int j = 0;

        // wrap, mirror, replicate
        for (; j != size_2; ++ j)
        {
            accumulator = BLACK;

            switch (border)
            {
                default:
                case BORDER_MIRROR:
                    for (k = - size_2; k != 0; ++ k)
                    {
                        color_accumulate(&accumulator, source_pixels[i * width + j - k], weights[k]);
                    }
                    break;

                case BORDER_CLAMP:
                    for (k = - size_2; k != 0; ++ k)
                    {
                        color_accumulate(&accumulator, source_pixels[i * width], weights[k]);
                    }
                    break;

                case BORDER_WRAP:
                    for (k = - size_2; k != 0; ++ k)
                    {
                        color_accumulate(&accumulator, source_pixels[(i + 1) * width + j - k], weights[k]);
                    }
                    break;
            }

            for (; k <= size_2; ++ k)
            {
                color_accumulate(&accumulator, source_pixels[i * width + j + k], weights[k]);
            }

            target_pixels[i * width + j] = accumulator;
        }

        for (; j != width - size_2; ++ j)
        {
            accumulator = BLACK;

            for (k = - size_2; k <= size_2; ++ k)
            {
                color_accumulate(&accumulator, source_pixels[i * width + j - k], weights[k]);
            }

            target_pixels[i * width + j] = accumulator;
        }

        for (; j != width; ++ j)
        {
            accumulator = BLACK;

            for (k = - size_2; k <= 0; ++ k)
            {
                color_accumulate(&accumulator, source_pixels[i * width + j + k], weights[k]);
            }

            switch (border)
            {
                case BORDER_BLACK:
                    break;

                case BORDER_MIRROR:
                    for (; k <= size_2; ++ k)
                    {
                        color_accumulate(&accumulator, source_pixels[i * width + j - k], weights[k]);
                    }
                    break;

                case BORDER_CLAMP:
                    for (; k <= size_2; ++ k)
                    {
                        color_accumulate(&accumulator, source_pixels[i * width], weights[k]);
                    }
                    break;

                case BORDER_WRAP:
                    for (; k <= size_2; ++ k)
                    {
                        color_accumulate(&accumulator, source_pixels[(i - 1) * width + j - k], weights[k]);
                    }
                    break;
            }

            target_pixels[i * width + j] = accumulator;
        }
    }

    return target;
}

Image * image_fast_area_sum(Image const * source)
{
    Image_Format format = source->format;

    error_check(format_to_size(format.format) != 1, "image is not single component");
    error_check(format.type != GL_FLOAT, "image is float type");

    Image * target = image_new(format);

    float const * source_pixels = (float const *) source->pixels;
    float * target_pixels = (float *) target->pixels;

    Size size = format.size;
    int width = size.x;
    int height = size.y;
    
    int m = (int) log2((float) height);
    int n = (int) log2((float) width);
    
    for (int k = 0; k != n; ++ k)
    {
        int offset = 1 << k;
        for (int i = 0; i != height; ++ i)
        {
            for (int j = 0; j != width; ++ i)
            {
                target_pixels[i * width + j] = source_pixels[i * width + j] + source_pixels[i * width + j + offset];
            }
        }
    }

    for (int k = 0; k != m; ++ k)
    {
        int offset = 1 << k;
        for (int i = 0; i != height; ++ i)
        {
            for (int j = 0; j != width; ++ i)
            {
                target_pixels[i * width + j] = source_pixels[i * width + j] + source_pixels[i * width + j + offset];
            }
        }
    }

    return target;
}

void image_statistics_print(Image_Statistics stats)
{
    char const pattern[] = "{%g, %g, %g}";
    printf("mean color = "); color_print_pattern(stats.mean, pattern); puts("");
    printf("variance = ");   color_print_pattern(stats.var,  pattern); puts("");
    printf("min color = ");  color_print_pattern(stats.min,  pattern); puts("");
    printf("max color = ");  color_print_pattern(stats.max,  pattern); puts("");
}

float image_statistics_threshold(Image const * image, float threshold)
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "image_statistics_threshold type must be float");
    error_check(format.format != GL_RGB, "image_statistics_threshold format must be rgb");

    Color const * pixels = (Color const *) image->pixels;
    int const pixel_count = size_total(format.size);

    float error = 0.0;

    for (int i = 0; i != pixel_count; ++ i)
    {
        Color color = pixels[i];
        float mean = (color.r + color.g + color.b) / 3.0;

        if (mean <= threshold)
            continue;

        error += mean;
    }

    return (float) error / pixel_count;
}

Image_Statistics image_statistics(Image const * image)
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "only float type supported");

    int const pixel_count = size_total(format.size);

    switch (format.format)
    {
        case GL_LUMINANCE:
        {
            float accumulator = 0, accumulator2 = 0, min_value = FLT_MAX, max_value = -FLT_MAX;
            float const * pixels = (float const *) image->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                float value = pixels[i];

                accumulator += value;
                accumulator2 += value * value;

                min_value = fmin(min_value, value);
                max_value = fmax(max_value, value);
            }

            float factor = 1.0 / pixel_count;
            Color mean  = color_scale(WHITE, accumulator * factor);
            Color mean2 = color_scale(WHITE, accumulator2 * factor);
            Color variance = color_sub(mean2, color_square(mean));

            Image_Statistics stats = {mean, variance, color_scale(WHITE, min_value), color_scale(WHITE, max_value)};
            return stats;
        }

        case GL_RGB:
        {
            Color accumulator = BLACK, accumulator2 = BLACK, min_color = MAX_COLOR, max_color = MIN_COLOR;
            Color const * pixels = (Color const *) image->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                Color color = pixels[i];

                color_accumulate(&accumulator, color, 1);
                color_accumulate(&accumulator2, color_square(color), 1);

                min_color = color_min(min_color, color);
                max_color = color_max(max_color, color);
            }

            float factor = 1.0 / pixel_count;
            Color mean = color_scale(accumulator, factor);
            Color mean2 = color_scale(accumulator2, factor);
            Color variance = color_sub(mean2, color_square(mean));

            Image_Statistics stats = {mean, variance, min_color, max_color};
            return stats;
        }

        case GL_RGBA:
        {
            Color accumulator = BLACK, accumulator2 = BLACK, min_color = MAX_COLOR, max_color = MIN_COLOR;
            Color4 const * pixels = (Color4 const *) image->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                Color color = pixels[i].c;

                color_accumulate(&accumulator, color, 1);
                color_accumulate(&accumulator2, color_square(color), 1);

                min_color = color_min(min_color, color);
                max_color = color_max(max_color, color);
            }

            float factor = 1.0 / pixel_count;
            Color mean = color_scale(accumulator, factor);
            Color mean2 = color_scale(accumulator2, factor);
            Color variance = color_sub(mean2, color_square(mean));

            Image_Statistics stats = {mean, variance, min_color, max_color};
            return stats;
        }

        default:
            error_check(1, "only RGB[A] float supported");
            Image_Statistics stats = {BLACK, BLACK, BLACK, BLACK};
            return stats;
    }
}

void image_problems_print(Image_Problems problems)
{
    printf("# negative pixels = %s%d%s\n",     problems.neg ? ANSI_BG_RED : "", problems.neg, ANSI_RESET);
    printf("# infinite pixels = %s%d%s\n",     problems.inf ? ANSI_BG_RED : "", problems.inf, ANSI_RESET);
    printf("# not a number pixels = %s%d%s\n", problems.nan ? ANSI_BG_RED : "", problems.nan, ANSI_RESET);
}

Image_Problems image_problems(Image const * image)
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "only float type supported");

    int const pixel_count = size_total(format.size);
    int neg = 0, inf = 0, nan = 0;

    switch (format.format)
    {
        case GL_LUMINANCE:
        {
            float const * pixels = (float const *) image->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                float value = pixels[i];

                if (value < 0)    ++ neg;
                if (isinf(value)) ++ inf;
                if (isnan(value)) ++ nan;
            }

            break;
        }

        case GL_RGB:
        {
            Color const * pixels = (Color const *) image->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                Color color = pixels[i];

                if (color_is_negative(color)) ++ neg;
                if (color_is_inf(color)) ++ inf;
                if (color_is_nan(color)) ++ nan;
            }

            break;
        }

        case GL_RGBA:
        {
            Color4 const * pixels = (Color4 const *) image->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                Color color = pixels[i].c;

                if (color_is_negative(color)) ++ neg;
                if (color_is_inf(color)) ++ inf;
                if (color_is_nan(color)) ++ nan;
            }

            break;
        }

        default:
            error_check(1, "only RGB[A] float supported");
            break;
    }

    Image_Problems problems = {neg, inf, nan};
    return problems;
}

void image_correct_gamma(Image * image, float gamma)
{
    Image_Format format = image->format;
    error_check(format.type != GL_FLOAT, "only float type supported");
    error_check(format.format != GL_RGB && format.format != GL_RGBA, "only rgb[a] format supported");

    int pixel_count = image_format_pixels(format);

    if (format.format == GL_RGB)
    {
        Color * pixels = (Color *) image->pixels;

        for (int i = 0; i != pixel_count; ++ i)
        {
            pixels[i] = color_correct_gamma(pixels[i], gamma);
        }
    }
    else if (format.format == GL_RGBA)
    {
        Color4 * pixels = (Color4 *) image->pixels;

        for (int i = 0; i != pixel_count; ++ i)
        {
            pixels[i].c = color_correct_gamma(pixels[i].c, gamma);
        }
    }
}

Image * image_normals(Image const * image, float scale)
{
    Image_Format format = image->format;
    error_check(format.type != GL_FLOAT, "only float type supported");
    error_check(format.format != GL_LUMINANCE, "only luminance format supported");

    Size size = format.size;
    Size d = size_stride(size);
    float const * source = (float const *) image->pixels;

    format.format = GL_RGB;
    Image * new_image = image_new(format);
    Vector * target = (Vector *) new_image->pixels;

    for (int i = 0; i != size.y; ++ i)
    {
        int dy1 = i ==          0 ? (size.y - 1) * d.y : -d.y;
        int dy2 = i == size.y - 1 ? (1 - size.y) * d.y : +d.y;

        for (int j = 0; j != size.x; ++ j)
        {
            int dx1 = j ==          0 ? (size.x - 1) : -d.x;
            int dx2 = j == size.x - 1 ? (1 - size.x) : +d.x;

            int index = size_index(size, 0, i, j);

            Vector gradient = 
            {
                (source[index + dx2] - source[index + dx1]) * scale / 2.0,
                (source[index + dy2] - source[index + dy1]) * scale / 2.0,
                1
            };

            target[index] = vector_normalize(gradient);
        }
    }

    return new_image;
}

void image_remap(Image * image, float scale, float bias)
{
    Image_Format format = image->format;
    error_check(format.type != GL_FLOAT, "only float type supported");

    int n = size_total(format.size) * format_to_size(format.format);
    float * pixels = image->pixels;

    for (int i = 0; i != n; ++ i)
    {
        * pixels = (* pixels) * scale + bias;
        ++ pixels;
    }
}

void image_normalize(Image * image)
{
    Image_Statistics stats = image_statistics(image);

    float max_value = vector_max_component(* (Vector *) &stats.max);
    float min_value = vector_min_component(* (Vector *) &stats.min);

    float scale = 1.0 / (max_value - min_value);
    float bias = -min_value * scale;

    image_remap(image, scale, bias);
}

#if 0
Image * image_perlin(Size size, float alpha, int n)
{
    static float const beta = 2.0;

    Image_Format format = {GL_FLOAT, GL_LUMINANCE, size};
    Image * image = image_new(format);

    GLfloat * pixels = image->pixels;

    for (int i = 0; i != size.z; ++ i) { float z = (i + 0.5) / size.z;
    for (int j = 0; j != size.y; ++ j) { float y = (j + 0.5) / size.y;
    for (int k = 0; k != size.x; ++ k) { float x = (k + 0.5) / size.x;

        int index = size_index(size, i, j, k);
#if 0
        z = z;
        pixels[index] = 0.5 * PerlinNoise2D(x, y, alpha, beta, n) + 0.5;
#else
        pixels[index] = 0.5 * PerlinNoise3D(x, y, z, alpha, beta, n) + 0.5;
#endif

    }
    }
    }

    return image;
}
#endif

Image * image_bump_map(Image const * image, float scale)
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "texture type must be float");

    Image * luminance = image_extract(image, GL_LUMINANCE, GL_GREEN, GL_LUMINANCE);
    Image * normals = image_normals(luminance, scale);
    image_destroy(luminance);

    return normals;
}

#if 0
static Color4 color4_sub(Color4 c1, Color4 c2)
{
    Color4 c = {color_sub(c1.c, c2.c), 0}; 
    return c;
}
#endif

Color map_color(Color c)
{
    float max_value = 0;

    if (fabs(c.r) > fabs(max_value)) max_value = c.r;
    if (fabs(c.g) > fabs(max_value)) max_value = c.g;
    if (fabs(c.b) > fabs(max_value)) max_value = c.b;

    if (max_value < 0)
        return color_scale(RED, -max_value);

    if (max_value > 0)
        return color_scale(CYAN, max_value);

    return BLACK;
}

Image * image_diff(Image const * image1, Image const * image2)
{
    Image_Format format1 = image1->format;
    Image_Format format2 = image2->format;

    error_check(format1.format != GL_RGBA, "images must be of format RGBA");
    error_check(format1.type != GL_FLOAT && format2.type != GL_HALF_FLOAT_ARB, "images must be of type float or half");
    error_check(format1.format != format2.format, "images must have same format");
    error_check(format1.type   != format2.type,   "images must have same type");
    error_check(format1.size.x != format2.size.x, "images must have same size");
    error_check(format1.size.y != format2.size.y, "images must have same size");
    error_check(format1.size.z != format2.size.z, "images must have same size");

    Size size = format1.size;

    Image * diff_image = image_new(format1);

    if (format1.type == GL_FLOAT)
    {
        Color4 const * source1 = (Color4 const *) image1->pixels;
        Color4 const * source2 = (Color4 const *) image2->pixels;
        Color4 * target = (Color4 *) diff_image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            Color4 d = {map_color(color_sub(source1[index].c, source2[index].c)), 1};
            target[index] = d;
        }
    }

    if (format1.type == GL_HALF_FLOAT_ARB)
    {
        unsigned short const * source1 = (unsigned short const *) image1->pixels;
        unsigned short const * source2 = (unsigned short const *) image2->pixels;
        unsigned short * target = (unsigned short *) diff_image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            Color c1 =
            {
                half_to_float(source1[index * 4 + 0]),
                half_to_float(source1[index * 4 + 1]),
                half_to_float(source1[index * 4 + 2])
            };
            Color c2 =
            {
                half_to_float(source2[index * 4 + 0]),
                half_to_float(source2[index * 4 + 1]),
                half_to_float(source2[index * 4 + 2])
            };
            Color d = color_sub(c1, c2);
            d = map_color(d);

            target[index * 4 + 0] = half_from_float(d.r);
            target[index * 4 + 1] = half_from_float(d.g);
            target[index * 4 + 2] = half_from_float(d.b);
            target[index * 4 + 3] = source1[index * 4 + 3]; // should be 1
        }
    }

    return diff_image;
}

void image_accumulate(Image * target, Image const * source, float scale)
{
    Image_Format target_format = target->format;
    Image_Format source_format = source->format;
    
    error_check(target_format.format != GL_RGBA,  "images must be of format RGBA");
    error_check(target_format.type   != GL_FLOAT, "images must be of type float");

    error_check(source_format.format != GL_RGBA,  "images must be of format RGBA");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");

    error_check(source_format.size.x != target_format.size.x, "images must have same size");
    error_check(source_format.size.y != target_format.size.y, "images must have same size");
    error_check(source_format.size.z != target_format.size.z, "images must have same size");
    
    Size size = source_format.size;
    Color4 const * source_pixels = (Color4 const *) source->pixels;
    Color4 * target_pixels = (Color4 *) target->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);
        target_pixels[index] = color4_add_scaled(source_pixels[index], target_pixels[index], scale);
    }
}

void image_accumulate_masked(Image * target, Image const * mask, Image const * source, float scale)
{
    Image_Format target_format = target->format;
    Image_Format source_format = source->format;
    
    error_check(target_format.format != GL_RGBA,  "images must be of format RGBA");
    error_check(target_format.type   != GL_FLOAT, "images must be of type float");

    error_check(source_format.format != GL_RGBA,  "images must be of format RGBA");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");

    error_check(source_format.size.x != target_format.size.x, "images must have same size");
    error_check(source_format.size.y != target_format.size.y, "images must have same size");
    error_check(source_format.size.z != target_format.size.z, "images must have same size");
    
    Size size = source_format.size;
    Color4 const * source_pixels = (Color4 const *) source->pixels;
    Color4 * target_pixels = (Color4 *) target->pixels;

    unsigned short const * mask_pixels = (unsigned short const *) mask->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);
        if (mask_pixels[index])
            continue;

        target_pixels[index] = color4_add_scaled(source_pixels[index], target_pixels[index], scale);
    }
}

float color_max_channel(Color c)
{   
    return fmax(c.r, fmax(c.g, c.b));
}

void image_accumulate_convergence(Image * target, Image const * reference, Image const * source, float tolerance, int iteration)
{
    Image_Format target_format = target->format;
    Image_Format source_format = source->format;

//    error_check(target_format.format != GL_LUMINANCE,  "images must be of format RGBA");
//    error_check(target_format.type   != GL_UNSIGNED_BYTE, "images must be of type float");

    error_check(source_format.format != GL_RGBA,  "images must be of format RGBA");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");

    error_check(source_format.size.x != target_format.size.x, "images must have same size");
    error_check(source_format.size.y != target_format.size.y, "images must have same size");
    error_check(source_format.size.z != target_format.size.z, "images must have same size");

    Size size = source_format.size;
    Color4 const * reference_pixels = (Color4 const *) reference->pixels;
    Color4 const * source_pixels    = (Color4 const *) source->pixels;
    //unsigned char * target_pixels = (unsigned char *) target->pixels;
    //float * target_pixels = (float *) target->pixels;
    unsigned short * target_pixels = (unsigned short *) target->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);
        if (target_pixels[index])
            continue;

        if (color_similar(color_scale(source_pixels[index].c, 1.0 / iteration), reference_pixels[index].c, tolerance * color_max_channel(reference_pixels[index].c)))
            target_pixels[index] = iteration;
    }
}

void image_log(Image * image)
{
    Image_Format format = image->format;

    error_check(format.format != GL_RGB, "image_log image must be of format RGB, RGBA, or luminance");
    error_check(format.type != GL_FLOAT, "image_log image must be of type float");

    Size size = format.size;
    Color * pixels = (Color *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);
        pixels[index] = color_log(pixels[index]);
    }
}

void image_scale(Image * image, float scale)
{
    Image_Format format = image->format;

    error_check(format.format != GL_RGB && format.format != GL_RGBA && format.format != GL_LUMINANCE, "images must be of format RGB, RGBA, or luminance");
    error_check(format.type != GL_FLOAT, "images must be of type float");

    Size size = format.size;

    if (format.format == GL_RGBA)
    {
        Color4 * pixels = (Color4 *) image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            pixels[index].c = color_scale(pixels[index].c, scale);
        }
    }
    else if (format.format == GL_RGB)
    {
        Color * pixels = (Color *) image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            pixels[index] = color_scale(pixels[index], scale);
        }
    }
    else if (format.format == GL_LUMINANCE)
    {
        float * pixels = (float *) image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            pixels[index] *= scale;
        }
    }
}

void image_assign_divide(Image * image, Image const * denominator_image, unsigned short zero_denominator)
{
    Image_Format format = image->format;

    error_check(format.format != GL_RGBA && format.format != GL_LUMINANCE, "images must be of format RGBA or luminance");
    error_check(format.type != GL_FLOAT, "images must be of type float");

    Size size = format.size;

    unsigned short const * denominators = (unsigned short const *) denominator_image->pixels;

    if (format.format == GL_RGBA)
    {
        Color4 * pixels = (Color4 *) image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            unsigned short denominator = denominators[index] ? denominators[index] : zero_denominator;

            pixels[index].c = color_scale(pixels[index].c, 1.0 / denominator);
        }
    }
    else if (format.format == GL_LUMINANCE)
    {
        float * pixels = (float *) image->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            unsigned short denominator = denominators[index] ? denominators[index] : zero_denominator;

            pixels[index] /= (float) denominator;
        }
    }
}

Image * image_crop(Image const * source, Size position, Size size)
{
    Image_Format source_format = source->format;
    Size source_size = source_format.size;

    error_check(source_format.format != GL_RGBA && source_format.format != GL_RGB && source_format.format != GL_LUMINANCE, "images must be of format RGBA, RGB, or luminance");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");

    Image_Format target_format = source_format;
    target_format.size = size;
    Size target_size   = size;

    Image * target = image_new(target_format);

    switch (source_format.format)
    {
        case GL_LUMINANCE:
        {

            float const * source_pixels = (float const *) source->pixels;
            float       * target_pixels = (float       *) target->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int source_index = size_index(source_size, position.z + i, position.y + j, position.x + k);
                int target_index = size_index(target_size, i, j, k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }

        case GL_RGB:
        {
            Color const * source_pixels = (Color const *) source->pixels;
            Color       * target_pixels = (Color       *) target->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int source_index = size_index(source_size, position.z + i, position.y + j, position.x + k);
                int target_index = size_index(target_size, i, j, k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }

        case GL_RGBA:
        {
            Color4 const * source_pixels = (Color4 const *) source->pixels;
            Color4       * target_pixels = (Color4       *) target->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int source_index = size_index(source_size, position.z + i, position.y + j, position.x + k);
                int target_index = size_index(target_size, i, j, k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }
   }

    return target;
}

Image * image_pad_border(Image const * source, Size padding, Border border)
{
    Size source_size = source->format.size;

    Size position = {- padding.x, - padding.y, - padding.z};
    Size size = {source_size.x + 2*padding.x, source_size.y + 2*padding.y, source_size.z + 2*padding.z};

    return image_cut(source, position, size, border);
}

Image * image_pad(Image const * source, Size padding)
{
    Image_Format source_format = source->format;
    Size source_size = source_format.size;

    error_check(source_format.format != GL_RGBA && source_format.format != GL_LUMINANCE, "images must be of format RGBA or luminance");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");

    Size target_size =
    {
        source_size.x + 2 * padding.x,
        source_size.y + 2 * padding.y, 
        source_size.z + 2 * padding.z
    };

    Image_Format target_format = source_format;
    target_format.size = target_size;

    Image * target = image_new(target_format);

    switch (source_format.format)
    {
        case GL_LUMINANCE:
        {
            float const * source_pixels = (float const *) source->pixels;
            float       * target_pixels = (float       *) target->pixels;

            for (int i = 0; i != source_size.z; ++ i)
            for (int j = 0; j != source_size.y; ++ j)
            for (int k = 0; k != source_size.x; ++ k)
            {
                int source_index = size_index(source_size, i, j, k);
                int target_index = size_index(target_size, padding.z  + i, padding.y  + j, padding.x  + k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;

        }
        case GL_RGBA:
        {
            Color4 const * source_pixels = (Color4 const *) source->pixels;
            Color4       * target_pixels = (Color4       *) target->pixels;

            for (int i = 0; i != source_size.z; ++ i)
            for (int j = 0; j != source_size.y; ++ j)
            for (int k = 0; k != source_size.x; ++ k)
            {
                int source_index = size_index(source_size, i, j, k);
                int target_index = size_index(target_size, padding.z  + i, padding.y  + j, padding.x  + k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }
    }

    return target;
}

Image * image_cut(Image const * source, Size position, Size size, Border border)
{
    Image_Format source_format = source->format;
    Size source_size = source_format.size;

    error_check(source_format.format != GL_RGBA && source_format.format != GL_LUMINANCE, "images must be of format RGBA or luminance");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");


    Size target_size = size;
    Image_Format target_format = source_format;
    target_format.size = target_size;

    Image * target = image_new(target_format);

    image_cut_raw(source, position, size, border, target);

    return target;
}

void image_cut_raw(Image const * source, Size position, Size size, Border border, Image * target)
{
    Image_Format source_format = source->format;
    Size source_size = source_format.size;

    error_check(source_format.format != GL_RGBA && source_format.format != GL_RGB && source_format.format != GL_LUMINANCE, "images must be of format RGB, RGBA, or luminance");
    error_check(source_format.type   != GL_FLOAT, "images must be of type float");

    Size target_size = size;

    switch (source_format.format)
    {
        case GL_LUMINANCE:
        {
            float const * source_pixels = (float const *) source->pixels;
            float       * target_pixels = (float       *) target->pixels;

            for (int i = 0; i != target_size.z; ++ i)
            for (int j = 0; j != target_size.y; ++ j)
            for (int k = 0; k != target_size.x; ++ k)
            {
                int source_index = size_index_border(source_size, position.z + i, position.y + j, position.x + k, border);
                int target_index = size_index       (target_size, i, j, k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }

        case GL_RGB:
        {
            Color const * source_pixels = (Color const *) source->pixels;
            Color       * target_pixels = (Color       *) target->pixels;

            for (int i = 0; i != target_size.z; ++ i)
            for (int j = 0; j != target_size.y; ++ j)
            for (int k = 0; k != target_size.x; ++ k)
            {
                int source_index = size_index_border(source_size, position.z + i, position.y + j, position.x + k, border);
                int target_index = size_index       (target_size, i, j, k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }

        case GL_RGBA:
        {
            Color4 const * source_pixels = (Color4 const *) source->pixels;
            Color4       * target_pixels = (Color4       *) target->pixels;

            for (int i = 0; i != target_size.z; ++ i)
            for (int j = 0; j != target_size.y; ++ j)
            for (int k = 0; k != target_size.x; ++ k)
            {
                int source_index = size_index_border(source_size, position.z + i, position.y + j, position.x + k, border);
                int target_index = size_index       (target_size, i, j, k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }
    }
}

void image_fill_rectangle(Image * image, Size position, Size size, Color4 color)
{
    Image_Format format = image->format;
    Size image_size = format.size;

    error_check(format.format != GL_RGBA, "images must be of format RGBA");
    error_check(format.type != GL_FLOAT, "images must be of type float");

    Color4 * pixels = (Color4 *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(image_size, position.z + i, position.y + j, position.x + k);

        pixels[index] = color;
    }
}

void image_fill_all(Image * image, Color4 color)
{
    Size position = {0, 0, 0};
    Size size = image->format.size;

    image_fill_rectangle(image, position, size, color);
}

void image_copy_rectangle(Image * target, Image const * source, Size target_position, Size source_position, Size size)
{
    Image_Format source_format = source->format;
    Image_Format target_format = target->format;

    error_check(source_format.format != target_format.format, "images must be of same format");
    error_check(source_format.type   != target_format.type,   "images must be of same type");

    error_check(source_format.format != GL_RGBA && source_format.format != GL_RGB && source_format.format != GL_LUMINANCE, "images must be of format RGBA, RGB, or LUMINANCE");
    error_check(source_format.type != GL_FLOAT, "images must be of type float");

//    error_check(target_format.format != GL_RGBA, "images must be of format RGBA");
//    error_check(target_format.type != GL_FLOAT, "images must be of type float");

    Size source_size = source_format.size;
    Size target_size = target_format.size;

    switch (source_format.format)
    {
        case GL_RGBA:
        {
            Color4 const * source_pixels = (Color4 const *) source->pixels;
            Color4 * target_pixels = (Color4 *) target->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int source_index = size_index(source_size, source_position.z + i, source_position.y + j, source_position.x + k);
                int target_index = size_index(target_size, target_position.z + i, target_position.y + j, target_position.x + k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }

        case GL_RGB:
        {
            Color const * source_pixels = (Color const *) source->pixels;
            Color * target_pixels = (Color *) target->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int source_index = size_index(source_size, source_position.z + i, source_position.y + j, source_position.x + k);
                int target_index = size_index(target_size, target_position.z + i, target_position.y + j, target_position.x + k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }

        case GL_LUMINANCE:
        {
            float const * source_pixels = (float const *) source->pixels;
            float * target_pixels = (float *) target->pixels;

            for (int i = 0; i != size.z; ++ i)
            for (int j = 0; j != size.y; ++ j)
            for (int k = 0; k != size.x; ++ k)
            {
                int source_index = size_index(source_size, source_position.z + i, source_position.y + j, source_position.x + k);
                int target_index = size_index(target_size, target_position.z + i, target_position.y + j, target_position.x + k);

                target_pixels[target_index] = source_pixels[source_index];
            }
            break;
        }
    }
}

void image_paste(Image * target, Image const * source, Size target_position)
{
    Size source_position = {0, 0, 0};
    Size size = source->format.size;
         
    image_copy_rectangle(target, source, target_position, source_position, size);
}

Image * image_zoom(Image const * source, Size scale)
{
    Image_Format source_format = source->format;
    Size source_size = source_format.size;

    error_check(source_format.format != GL_RGBA, "images must be of format RGBA");
    error_check(source_format.type != GL_FLOAT, "images must be of type float");

    Size target_size = size_scale(source_size, scale);

    Image_Format target_format = source_format;
    target_format.size = target_size;

    Image * target = image_new(target_format);

    Color4 const * source_pixels = (Color4 const *) source->pixels;
    Color4 * target_pixels = (Color4 *) target->pixels;

    for (int i = 0; i != source_size.z; ++ i) for (int i2 = 0; i2 != scale.z; ++ i2)
    for (int j = 0; j != source_size.y; ++ j) for (int j2 = 0; j2 != scale.y; ++ j2)
    for (int k = 0; k != source_size.x; ++ k) for (int k2 = 0; k2 != scale.x; ++ k2)
    {
        int source_index = size_index(source_size, i, j, k);
        int target_index = size_index(target_size, i * scale.z + i2, j * scale.y + j2, k * scale.x + k2);

        target_pixels[target_index] = source_pixels[source_index];
    }

    return target;
}

void image_flip_horizontal(Image * image)
{
    Image_Format format = image->format;
    Size size = format.size;

    error_check(format.format != GL_RGBA, "images must be of format RGBA");
    error_check(format.type != GL_FLOAT, "images must be of type float");

    Color4 * pixels = (Color4 *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x / 2; ++ k)
    {
        int index_1 = size_index(size, i, j, k);
        int index_2 = size_index(size, i, j, size.x - 1 - k);

        swap(Color4, pixels[index_1], pixels[index_2]);
    }
}

void image_negate(Image * image)
{
    Image_Format format = image->format;
    Size size = format.size;

    error_check(format.format != GL_RGBA, "images must be of format RGBA");
    error_check(format.type != GL_FLOAT, "images must be of type float");

    Color4 * pixels = (Color4 *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);

        pixels[index].c = color_negate(pixels[index].c);
    }
}

Image * image_apply_palette(Image const * source, Image const * palette)
{
    Image_Format source_format = source->format;
    Size size = source_format.size;

    Image_Format palette_format = palette->format;

    error_check(source_format.format != GL_LUMINANCE, "images must be of format luminance");
    error_check(source_format.type   != GL_FLOAT,     "images must be of type float");

    Image_Format target_format = source_format;
    target_format.format = GL_RGBA;
    Image * target = image_new(target_format);

    float const * source_pixels = (float const *) source->pixels;
    Color4 * target_pixels = (Color4 *) target->pixels;

    if (palette_format.format == GL_RGBA)
    {
        Color4 const * palette_pixels = (Color4 const *) palette->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            float luminance = clamp(source_pixels[index]);
            int color_index = luminance * (palette->format.size.x - 1);

            target_pixels[index] = palette_pixels[color_index];
        }
    }
    else if (palette_format.format == GL_RGB)
    {
        Color const * palette_pixels = (Color const *) palette->pixels;

        for (int i = 0; i != size.z; ++ i)
        for (int j = 0; j != size.y; ++ j)
        for (int k = 0; k != size.x; ++ k)
        {
            int index = size_index(size, i, j, k);
            float luminance = clamp(source_pixels[index]);
            int color_index = luminance * (palette->format.size.x - 1);

            target_pixels[index].c = palette_pixels[color_index];
            target_pixels[index].a = 1.0;
        }
    }

    return target;
}

int image_count_zeros(Image const * image)
{
    Image_Format format = image->format;
    Size size = format.size;

    int zeros = 0;

    error_check(format.format != GL_LUMINANCE,      "images must be of format luminance");
    error_check(format.type   != GL_UNSIGNED_SHORT, "images must be of type unsigned short");

    unsigned short const * pixels = (unsigned short const *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);
        if (! pixels[index])
            ++ zeros;
    }

    return zeros;
}

int image_sum_values(Image const * image)
{
    Image_Format format = image->format;
    Size size = format.size;

    int value = 0;

    error_check(format.format != GL_LUMINANCE,      "images must be of format luminance");
    error_check(format.type   != GL_UNSIGNED_SHORT, "images must be of type unsigned short");

    unsigned short const * pixels = (unsigned short const *) image->pixels;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);
        value += pixels[index];
    }

    return value;
}

Image * image_blend(Image const * image_1, float t1, Image const * image_2, float t2)
{
    error_check(! image_format_equal(image_1->format, image_2->format), "image to blend must have same image format");
    error_check(image_1->format.type   != GL_FLOAT,     "image to blend must have type float");
    //error_check(image_1->format.size.z != 1 && image_2->format.size.z != 1 || image_1->format.size.z != image_2->format.size.z, "one of the image depths must be 1 or they must have the same depth");
    // TODO support single depth

    Image_Format format = image_1->format;
    Size size  = format.size;

    Image * image = image_new(format);

    if (format.format == GL_LUMINANCE_ALPHA)
    {
        float const * pixels_1 = (float const *) image_1->pixels;
        float const * pixels_2 = (float const *) image_2->pixels;
        float * pixels = (float *) image->pixels;

        int count = size_total(size);

        for (int i = 0; i != count; ++ i)
        {
            pixels[i * 2 + 0] = pixels_1[i * 2] * t1 + pixels_2[i * 2] * t2;
            pixels[i * 2 + 1] = 1.0;
        }
    }
    else if (format.format == GL_RGBA)
    {
        Color4 const * pixels_1 = (Color4 const *) image_1->pixels;
        Color4 const * pixels_2 = (Color4 const *) image_2->pixels;
        Color4 * pixels = (Color4 *) image->pixels;

        int count = size_total(size);

        for (int i = 0; i != count; ++ i)
        {
            pixels[i].c = color_blend(pixels_1[i].c, t1, pixels_2[i].c, t2);
            pixels[i].a = 1.0;
        }
    }
    else
    {
        float const * pixels_1 = (float const *) image_1->pixels;
        float const * pixels_2 = (float const *) image_2->pixels;
        float * pixels = (float *) image->pixels;

        int count = size_total(size) * format_to_size(format.format);

        for (int i = 0; i != count; ++ i)
        {
            pixels[i] = pixels_1[i] * t1 + pixels_2[i] * t2;
        }
    }

    return image;
}

void image_assign_square(Image * image)
{
    error_check(image->format.type != GL_FLOAT, "image to square must have type float");

    Image_Format format = image->format;
    Size size = format.size;

    if (format.format == GL_RGBA)
    {
        int count = size_total(size);
        Color4 * pixels = (Color4 *) image->pixels;
        
        for (int i = 0; i != count; ++ i)
        {
            pixels[i].c = color_square(pixels[i].c);
        }
    }
    else if (format.format == GL_LUMINANCE_ALPHA)
    {
        int count = size_total(size);
        float * pixels = (float *) image->pixels;

        for (int i = 0; i != count; ++ i)
        {
            pixels[i * 2] = SQ(pixels[i * 2]);
        }
    }
    else
    {
        int count = size_total(size) * format_to_size(format.format);
        float * pixels = (float *) image->pixels;

        for (int i = 0; i != count; ++ i)
        {
            pixels[i] = SQ(pixels[i]);
        }
    }
}

Image * image_divide(Image const * source_1, Image const * source_2, float clamp_value)
{
    error_check(! image_format_equal(source_1->format, source_2->format), "image to divide must have same image format");
    error_check(source_1->format.type   != GL_FLOAT,     "image to divide must have type float");

    Image_Format format = source_1->format;
    Size size = format.size;

    Image * target = image_new(format);

    if (format.format == GL_RGBA)
    {
        Color min_color = {clamp_value, clamp_value, clamp_value};
        int count = size_total(size);
        Color4 const * source_pixels_1 = (Color4 const *) source_1->pixels;
        Color4 const * source_pixels_2 = (Color4 const *) source_2->pixels;
        Color4       * target_pixels   = (Color4       *) target->pixels;

        for (int i = 0; i != count; ++ i)
        {
            target_pixels[i].c = color_div(source_pixels_1[i].c, color_clamp_min(source_pixels_2[i].c, min_color));
            target_pixels[i].a = 1;
        }
    }
    else if (format.format == GL_LUMINANCE_ALPHA)
    {
        int count = size_total(size);
        float const * source_pixels_1 = (float const *) source_1->pixels;
        float const * source_pixels_2 = (float const *) source_2->pixels;
        float       * target_pixels   = (float       *) target->pixels;

        for (int i = 0; i != count; ++ i)
        {
            target_pixels[i * 2] = source_pixels_1[i * 2] / clamp_to(source_pixels_2[i * 2], clamp_value, FLT_MAX);
        }
    }
    else
    {
        int count = size_total(size) * format_to_size(format.format);
        float const * source_pixels_1 = (float const *) source_1->pixels;
        float const * source_pixels_2 = (float const *) source_2->pixels;
        float       * target_pixels   = (float       *) target->pixels;

        for (int i = 0; i != count; ++ i)
        {
            target_pixels[i] = source_pixels_1[i] / clamp_to(source_pixels_2[i], clamp_value, FLT_MAX);
        }
    }

    return target;
}

void image_clean_nan(Image * image)
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "image to clean must have type float");

    Size size = format.size;

    int count = size_total(size) * format_to_size(format.format);
    float * pixels = (float *) image->pixels;

    for (int i = 0; i != count; ++ i)
    {
        if (isnan(pixels[i]))
            pixels[i] = 0;
    }
}

Image * image_merge(Image const * const * images, int count)
{
    Image_Format merged_format = images[0]->format;

    for (int i = 1; i != count; ++ i)
    {
        Image_Format format = images[i]->format;
        Size size = format.size;

        error_check(merged_format.type   != format.type ||
                    merged_format.format != format.format, "format and type of images must be equal");
        error_check(merged_format.size.x != size.x ||
                    merged_format.size.y != size.y, "X and Y dimensions of images must be equal");

        merged_format.size.z += size.z;
    }

    Image * image = image_new(merged_format);
    char * pixels = (char *) image->pixels;

    for (int i = 0; i != count; ++ i)
    {
        Image_Format format = images[i]->format;
        int byte_count = image_format_bytes(format);

        memcpy(pixels, images[i]->pixels, byte_count);
        pixels += byte_count;
    }

    return image;
}

float image_mse(Image const * source, Image const * reference, int relative)
{
    Image * squared_error = image_squared_error(source, reference, relative);

    Image_Format format = squared_error->format;
    //int pixel_count = image_format_pixels(format);
    int count = format_to_size(format.format) * size_total(format.size);

    float acc = 0;

    if (format.type == GL_FLOAT)
    {
        float const * pixels = (float const *) squared_error->pixels;
        for (int i = 0; i != count; ++ i) {acc += pixels[i];}
    }
    else if (format.type == GL_UNSIGNED_BYTE)
    {
        unsigned char const * pixels = (unsigned char const *) squared_error->pixels;
        for (int i = 0; i != count; ++ i) {acc += pixels[i];}
    }
    else
    {
        error_fail("image_mse: unsupported type");
    }

    return acc / count;
}

float image_rmse(float mse) { return sqrt(mse) * 255; }
float image_psnr(float mse) { return - 10 * log(mse) / log(10.0); }

#if 0
Image * image_ssim(Image const * sources, Image const * reference)
{
    float L  = 1.0;
    float k1 = 0.01;
    float k2 = 0.02;
    float gaussian = 1;

    float c1 = SQ(k1 * L);
    float c2 = SQ(k2 * L);

    float n;
    float * k;
    if (gaussian)
    {
        n = 11;
        k = // kernel
    }
    else
    {
        n = 8;
        k = // kernel
    }

    mx2 = image_square(mx);
    my2 = image_square(my);
    mx_my = image_multiply(mx, my);

    sx2 = // conv
    sy2 = // conv
    sxz = // conv

    Image * ssim = .... // sum and divide images

    return ssim;
}

float image_mssim(Image const * sources, Image const * reference)
{
    Image * ssim = image_ssim(sources, reference);

    // ...
    return 42;
}

float image_dmssim(float mssim) { return 1.0 / mssim - 1.0; }
#endif


Image * image_squared_error(Image const * sources, Image const * reference, int relative)
{
    Image * difference = image_subtract(sources, reference);
    Image * error;

    if (relative)
    {
        static float const clamp_value = 0.01;
        error = image_divide(difference, reference, clamp_value);
        image_destroy(difference);
    }
    else
        error = difference;

    image_assign_square(error);

    return error;
}

Image * image_squared_error_visual(Image const * sources, Image const * reference)
{
    //Image * error = image_squared_error(sources, reference, 1);
    Image * error = image_subtract(sources, reference);
    image_assign_square(error);

    Image * laplacian = kernel_laplacian_2();
    Image * edge = image_new(reference->format);
    image_splat(edge, reference, laplacian, 0);
    image_assign_square(edge);

    static float const clamp_value = 0.0001;
    Image * error2 = image_divide(error, edge, clamp_value);

    image_destroy(edge);
    image_destroy(laplacian);
    image_destroy(error);

    return error2;
}

Image * image_laplacian_error(Image const * sources, Image const * reference, int relative)
{
    Image * laplacian = kernel_laplacian_2();
    Image * laplacian_1 = image_new(sources->format);
    Image * laplacian_2 = image_new(reference->format);

    image_splat(laplacian_1, sources,   laplacian, 0);
    image_splat(laplacian_2, reference, laplacian, 0);

    image_assign_square(laplacian_1);
    image_assign_square(laplacian_2);

    Image * error = image_squared_error(laplacian_1, laplacian_2, relative);

    image_destroy(laplacian_1);
    image_destroy(laplacian_2);
    image_destroy(laplacian);

    return error;
}

Image * image_select_bandwidth(Image const * error)
{
    error_check(error->format.type != GL_FLOAT  || error->format.format != GL_RGB, "error format and type must be rgb and float");

    Image_Format error_format = error->format;
    Size error_size = error_format.size;

    Image_Format target_format = {GL_UNSIGNED_BYTE, GL_LUMINANCE, {error_size.x, error_size.y, 1}};
    Image * target = image_new(target_format);
    Size target_size = target_format.size;

    Color const   * error_pixels  = (Color const   *) error->pixels;
    unsigned char * target_pixels = (unsigned char *) target->pixels;

    for (int i = 0; i != error_size.y; ++ i)
    for (int j = 0; j != error_size.x; ++ j)
    {
        float min_error = FLT_MAX;
        unsigned char min_index = 255;

        for (int k = 0; k != error_size.z; ++ k)
        {
            int   error_index = size_index(error_size, k, i, j);
            Color error_pixel = error_pixels[error_index];

            float e = error_pixel.r + error_pixel.g + error_pixel.b;
            if (e < min_error)
            {
                min_error = e;
                min_index = k;
            }

            int target_index = size_index(target_size, 0, i, j);
            target_pixels[target_index] = min_index;
        }
    }

    return target;
}

Image * image_filter(Image const * source, Image const * selected_bandwidths)
{
    error_check(source->format.type != GL_FLOAT  || source->format.format != GL_RGB, "image format and type must be rgb and float");
    error_check(selected_bandwidths->format.type != GL_UNSIGNED_BYTE  || selected_bandwidths->format.format != GL_LUMINANCE, "selected bandwidth format and type must be luminance and unsigned byte");

    Image_Format source_format = source->format;
    Size source_size = source_format.size;
    Color const * source_pixels = (Color const *) source->pixels;

    Size bandwidth_size = selected_bandwidths->format.size;
    unsigned char const * bandwidths = (unsigned char const *) selected_bandwidths->pixels;

    Image_Format target_format = source_format;
    target_format.size.z = 1;
    Size target_size = target_format.size;
    Image * target = image_new(target_format);
    Color * target_pixels = (Color *) target->pixels;

    for (int i = 0; i != target_size.y; ++ i)
    for (int j = 0; j != target_size.x; ++ j)
    {
        int bandwidth_index = size_index(bandwidth_size, 0, i, j);
        int k = bandwidths[bandwidth_index];

        int source_index = size_index(source_size, k, i, j);
        int target_index = size_index(target_size, 0, i, j);
        target_pixels[target_index] = source_pixels[source_index];
    }

    return target;
}

Image * image_sum(Image const * source)
{
    Image_Format source_format = source->format;
    Size source_size = source_format.size;
    Color const * source_pixels = (Color const *) source->pixels;

    error_check(source_format.type != GL_FLOAT, "image_sum: type must be float");
    error_check(source_format.format != GL_RGB, "image_sum: format must be rgb");

    Image_Format target_format = source_format;
    target_format.size.x = 1;
    target_format.size.y = 1;
    Size target_size = target_format.size;
    Image * target = image_new(target_format);
    Color * target_pixels = (Color *) target->pixels;

    for (int k = 0; k != source_size.z; ++ k)
    {
        Color accumulator = BLACK;

        for (int i = 0; i != source_size.y; ++ i)
        for (int j = 0; j != source_size.x; ++ j)
        {
            int source_index = size_index(source_size, 0, i, j);
            color_accumulate(&accumulator, source_pixels[source_index], 1);
        }

        int target_index = size_index(target_size, k, 0, 0);
        target_pixels[target_index] = accumulator;
    }

    return target;
}

Image * image_duplicate_layers(Image const * source, int layer_count)
{
    error_check(source->format.size.z != 1, "source image must have single layer");
    error_check(layer_count < 1, "target image must have at least 1 layer");

    Image_Format source_format = source->format;
    int byte_count = image_format_bytes(source_format);

    Image_Format target_format = source_format;
    target_format.size.z = layer_count;
    Image * target = image_new(target_format);
    char * target_pixels = (char *) target->pixels;

    for (int i = 0; i != layer_count; ++ i)
    {
        memcpy(target_pixels, source->pixels, byte_count);
        target_pixels += byte_count;
    }

    return target;
}

static Color filter_mean(Image const * image, Image const * kernel, int x, int y)
{
    Size  image_size =  image->format.size;
    Size kernel_size = kernel->format.size;

    Color accumulator = BLACK;

    for (int i = 0; i != kernel_size.y; ++ i)
    for (int j = 0; j != kernel_size.x; ++ j)
    {
        int  pixel_index = size_index( image_size, 0, y + i, x + j);
        int weight_index = size_index(kernel_size, 0, i, j);

        color_accumulate(&accumulator,
            ((Color const *)  image->pixels)[pixel_index],
            ((float const *) kernel->pixels)[weight_index]);
    }

    return accumulator;
}

static Color filter_std_dev(Image const * image, Image const * kernel, int x, int y, Color mean)
{
    Size  image_size =  image->format.size;
    Size kernel_size = kernel->format.size;

    Color accumulator = BLACK;

    for (int i = 0; i != kernel_size.y; ++ i)
    for (int j = 0; j != kernel_size.x; ++ j)
    {
        int  pixel_index = size_index( image_size, 0, y + i, x + j);
        int weight_index = size_index(kernel_size, 0, i, j);

        Color value = color_sub(((Color const *) image->pixels)[pixel_index], mean);
        color_accumulate(&accumulator, color_square(value), ((float const *) kernel->pixels)[weight_index]);
    }

    return color_sqrt(accumulator);
}

static Color filter_std_dev_12(Image const * image_1, Image const * image_2, Image const * kernel, int x, int y, Color mean_1, Color mean_2)
{
    Size  image_size = image_1->format.size;
    Size kernel_size =  kernel->format.size;

    Color accumulator = BLACK;

    for (int i = 0; i != kernel_size.y; ++ i)
    for (int j = 0; j != kernel_size.x; ++ j)
    {
        int  pixel_index = size_index( image_size, 0, y + i, x + j);
        int weight_index = size_index(kernel_size, 0, i, j);

        Color value_1 = color_sub(((Color const *) image_1->pixels)[pixel_index], mean_1);
        Color value_2 = color_sub(((Color const *) image_2->pixels)[pixel_index], mean_2);

        color_accumulate(&accumulator, color_multiply(value_1, value_2, 1), ((float const *) kernel->pixels)[weight_index]);
    }

    return accumulator;
}

Color image_mssim(Image const * image_1, Image const * image_2, float L, float k_1, float k_2, float alpha, float beta, float gamma)
{
    error_check(! image_format_equal(image_1->format, image_2->format), "MSSIM requires both images to have same format");
    error_check(image_1->format.type != GL_FLOAT, "MSSIM image type must be float");
    error_check(image_1->format.format != GL_RGB, "MSSIM image format must be rgb");
    error_check(image_1->format.size.z != 1, "MSSIM image depth must be 1");

#if 0
    static int const kernel_width = 11;
#else
    static int const kernel_width = 5;
#endif

    float c_1 = SQ(k_1 * L);
    float c_2 = SQ(k_2 * L);
    float c_3 = c_2 / 2;

    Color C_1 = {c_1, c_1, c_1};
    Color C_2 = {c_2, c_2, c_2};
    Color C_3 = {c_3, c_3, c_3};

#if 1
    static float const sigma = 1.5;
    Image * kernel = kernel_gaussian_2d(kernel_width, sigma, ORIGIN);
#else
    Image * kernel = kernel_box_2d(kernel_width, ORIGIN);
#endif

    Size size = image_1->format.size;
    Size map_size = {size.x - kernel_width, size.y - kernel_width, 1};

    Image_Format format = {GL_FLOAT, GL_RGB, map_size};
    Image * ssim_map = image_new(format);
    Color * pixels = (Color *) ssim_map->pixels;

    Color m_1, m_2, s_1, s_2, s_12, n1, n2, n3, d1, d2, d3;
    int i, j, index;
#ifdef OMP
    #pragma omp parallel for private(i, j, index, m_1, m_2, s_1, s_2, s_12, n1, n2, n3, d1, d2, d3)
#endif
    for (i = 0; i < map_size.y; ++ i)
    {
        for (j = 0; j < map_size.x; ++ j)
        {
            m_1  = filter_mean(image_1, kernel, j, i);
            m_2  = filter_mean(image_2, kernel, j, i);
            s_1  = filter_std_dev(image_1, kernel, j, i, m_1);
            s_2  = filter_std_dev(image_2, kernel, j, i, m_2);
            s_12 = filter_std_dev_12(image_1, image_2, kernel, j, i, m_1, m_2);

            n1 = color_add(color_multiply(m_1, m_2, 2), C_1);
            d1 = color_add_3(color_square(m_1), color_square(m_2), C_1);

#if 0
            n2 = color_add(color_scale(s_12, 2), C_2);
            d2 = color_add_3(color_square(s_1), color_square(s_2), C_2);
#else
            n2 = color_add(color_scale(color_mul(s_1, s_2), 2), C_2);
            d2 = color_add_3(color_square(s_1), color_square(s_2), C_2);

            n3 = color_add(s_12, C_3);
            d3 = color_add(color_mul(s_1, s_2), C_3);
#endif

            index = size_index(map_size, 0, i, j);
#if 0
            pixels[index] = color_div(color_mul(n1, n2), color_mul(d1, d2));
#else
            Color a = color_pow(color_div(n1, d1), alpha);
            Color b = color_pow(color_div(n2, d2), beta);
            Color c = color_pow(color_div(n3, d3), gamma);

            pixels[index] = color_mul(color_mul(a, b), c);
#endif
        }
    }

    Color accumulator = BLACK;

    for (i = 0; i != map_size.y; ++ i)
    for (j = 0; j != map_size.x; ++ j)
    {
        index = size_index(map_size, 0, i, j);
        color_accumulate(&accumulator, pixels[index], 1.0);
    }

    image_destroy(ssim_map);
    image_destroy(kernel);

    int pixel_count = map_size.x * map_size.y;
    return color_scale(accumulator, 1.0 / pixel_count);
}

float color_squared_relative_error(Color ref, Color c)
{
    Color d = color_sub(c, ref);

    d.r /= (ref.r + 0.001);
    d.g /= (ref.g + 0.001);
    d.b /= (ref.b + 0.001);

    return (SQ(d.r) + SQ(d.g) + SQ(d.b)) / 3.0;
}

float image_similar(Image const * reference, Image const * image, float tolerance)
{
    error_check(! image_format_equal(reference->format, image->format), "image_similar requires both images to have same format");
    error_check(reference->format.type != GL_FLOAT, "image_similar image type must be float");
    error_check(reference->format.format != GL_RGB, "image_similar image format must be rgb");

    Image_Format format = reference->format;
    Size size = format.size;

    Color const * reference_pixels = (Color const *) reference->pixels;
    Color const * image_pixels = (Color const *) image->pixels;

    int not_converged = 0;

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);

        float error = color_squared_relative_error(reference_pixels[index], image_pixels[index]);

        if (error > tolerance)
            ++ not_converged;
    }

    int pixel_count = size_total(size);
    return (float) not_converged / pixel_count;
}

float * image_error_distribution(Image const * image, int n, int logarithmic)
{
    error_check(image->format.type != GL_FLOAT, "image_error_distribution image type must be float");
    error_check(image->format.format != GL_RGB, "image_error_distribution image format must be rgb");

    Size size = image->format.size;
    Color const * pixels = (Color const *) image->pixels;

    float * distribution = calloc_array(float, n);

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);

        Color color = pixels[index];
        float error = (color.r + color.g + color.b) / 3.0;

        //float value = - log(error) / log(10.0);
        float value = logarithmic ? - log(error) / log(2.0) : error / 0.0003;

        int error_index;
        if (value < 0)
            error_index = 0;
        else if (value >= n)
            error_index = n - 1;
        else
            error_index = (int) value;

        distribution[error_index] += 1.0;
    }

    int pixel_count = size_total(size);
    for (int i = 0; i != n; ++ i)
    {
        distribution[i] /= pixel_count;
    }

    return distribution;
}

Image * image_to_lab(Image const * source)
{
    error_check(source->format.type != GL_FLOAT, "image_to_lab image type must be float");
    error_check(source->format.format != GL_RGB, "image_to_lab image format must be rgb");

    Image_Format format = source->format;
    Size size = format.size;

    Image * target = image_new(format);

    Color const * source_pixels = (Color const *) source->pixels;
    Color       * target_pixels = (Color       *) target->pixels;

    Vector white_point = color_to_xyz(WHITE);

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);

        Color rgb = source_pixels[index];
        Vector xyz = color_to_xyz(rgb);
        Vector lab = color_xyz_to_lab(xyz, white_point);

        Color c = {lab.x, lab.y, lab.z};
        target_pixels[index] = c;
    }

    return target;
}

void image_splat(Image * target, Image const * source, Image const * kernel, int rescale)
{
    error_check(! image_format_equal(target->format, source->format), "image_splat requires target and source to have same format");
    error_check(target->format.type != GL_FLOAT, "image_splat type must be luminance");
    error_check(target->format.format != GL_RGB, "image_splat format must be rgb");
    error_check(kernel->format.type != GL_FLOAT, "image_splat kernel type must be float");
    error_check(kernel->format.format != GL_LUMINANCE, "image_splat kernel format must be luminance");

    Size target_size = target->format.size;
    Size source_size = source->format.size;
    Size kernel_size = kernel->format.size;
    Size kernel_half_size = {kernel_size.x / 2, kernel_size.y / 2, 1};

    Color       * target_pixels = (Color       *) target->pixels;
    Color const * source_pixels = (Color const *) source->pixels;
    float const * kernel_pixels = (float const *) kernel->pixels;

    // add omp
    for (int i = 0; i != target_size.z; ++ i)
    for (int j = 0; j != target_size.y; ++ j)
    {
        int k;
#ifdef OMP
    #pragma omp parallel for
#endif
    for (k = 0; k < target_size.x; ++ k)
    {
        int target_index = size_index(target_size, i, j, k);

        Color accumulator = BLACK;
        float accumulated_weight = 0.0;

        for (int l = 0; l != kernel_size.y; ++ l)
        for (int m = 0; m != kernel_size.x; ++ m)
        {
            int n = j + l - kernel_half_size.y;
            int o = k + m - kernel_half_size.x;

            if (n < 0 || n >= source_size.y) continue;
            if (o < 0 || o >= source_size.x) continue;

            int kernel_index = size_index(kernel_size, 0, l, m);
            int source_index = size_index(source_size, i, n, o);

            float weight = kernel_pixels[kernel_index];

            color_accumulate(&accumulator, source_pixels[source_index], weight);
            accumulated_weight += weight;
        }

        if (rescale)
            target_pixels[target_index] = color_scale(accumulator, 1.0 / accumulated_weight);
        else
            target_pixels[target_index] = accumulator;
    }
    }
}

void image_update_mean(Image * mean_n, Image const * x_n1, int n)
{
    error_check(! image_format_equal(mean_n->format, x_n1->format), "image_update_mean must have same format");
    error_check(mean_n->format.type != GL_FLOAT, "image_update_mean type must be float");
    error_check(mean_n->format.format != GL_RGB, "image_update_mean format must be rgb");

    Size size = mean_n->format.size;

    Color       * target_pixels = (Color       *) mean_n->pixels;
    Color const * source_pixels = (Color const *) x_n1->pixels;

    float scale = 1.0 / (n + 1);

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);

        target_pixels[index] = color_scale(color_add(color_scale(target_pixels[index], n), source_pixels[index]), scale);
    }
}

void image_update_variance(Image * var_n, Image const * mean_n1, Image const * x_n1, int n)
{
    error_check(! image_format_equal(var_n->format, x_n1->format), "image_update_mean must have same format");
    error_check(var_n->format.type != GL_FLOAT, "image_update_mean type must be float");
    error_check(var_n->format.format != GL_RGB, "image_update_mean format must be rgb");

    Size size = var_n->format.size;

    Color       *     var_pixels = (Color       *) var_n->pixels;
    Color const * mean_n1_pixels = (Color const *) mean_n1->pixels;
    Color const *    x_n1_pixels = (Color const *) x_n1->pixels;

    // biased variance
    if (n == 0)
    {
        image_clear(var_n);
        return;
    }

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int index = size_index(size, i, j, k);

        Color a = color_scale(var_pixels[index], (float) n / (n + 1));
        Color b = color_scale(color_square(color_sub(x_n1_pixels[index], mean_n1_pixels[index])), 1.0 / n);
        var_pixels[index] = color_add(a, b);
    }
}

static int compare(void const * data_1, void const * data_2)
{
    float value_1 = * ((float const *) data_1);
    float value_2 = * ((float const *) data_2);

    float delta = value_1 - value_2;
    return signum(delta);
}

static void initialize_offsets(int offsets[], int width, Size stride)
{
    for (int i = 0; i != width; ++ i)
    for (int j = 0; j != width; ++ j)
    {
        int k = i - width/2;
        int l = j - width/2;

        offsets[i*width + j] = stride.x * l + stride.y * k;
    }
}

Image * image_filter_median(Image const * source, int width, int median_index)
{
    error_check(source->format.type != GL_FLOAT, "imean_filter_median type must be float");
    error_check(source->format.format != GL_RGB, "image_filter_median format must be rgb");
    error_check((width % 2) == 0, "median filter size must be odd");
    error_check(median_index >= 0 && median_index < SQ(width) - 1, "median index must be in [0, w^2)");

    int width_2 = width / 2;

    Image_Format format = source->format;
    Image * target = image_new(format);

    float const * source_pixels = (float const *) source->pixels;
    float       * target_pixels = (float       *) target->pixels;

    Size size   = format.size;
    Size stride = size_stride(size);

    int * offsets = malloc_array(int, width * width);
    initialize_offsets(offsets, width, stride);
    float * buffer = malloc_array(float, width * width);

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int target_index = size_index(size, i, j, k);

        for (int l = 0; l != 3; ++ l)
        {
            int count = 0;

            for (int m = -width_2; m <= width_2; ++ m)
            {
                if (j + m < 0 || j + m >= size.y) continue;

                for (int n = -width_2; n <= width_2; ++ n)
                {
                    if (k + n < 0 || k + n > size.x) continue;
                    
                    buffer[count ++] = source_pixels[(target_index + offsets[(width_2 + m) * width + width_2 + n]) * 3 + l];
                }
            }

            qsort(buffer, count, sizeof(float), compare);

            target_pixels[target_index * 3 + l] = buffer[median_index];
        }
    }

    free(offsets);
    free(buffer);

    return target;
}

Image * image_filter_median_weighted(Image const * source)
{
    int const width = 3;
    int median_index = 7;

    error_check(source->format.type != GL_FLOAT, "imean_filter_median type must be float");
    error_check(source->format.format != GL_RGB, "image_filter_median format must be rgb");
    error_check((width % 2) == 0, "median filter size must be odd");

    int width_2 = width / 2;

    Image_Format format = source->format;
    Image * target = image_new(format);

    float const * source_pixels = (float const *) source->pixels;
    float       * target_pixels = (float       *) target->pixels;

    Size size   = format.size;
    Size stride = size_stride(size);

    int * offsets = malloc_array(int, width * width);
    initialize_offsets(offsets, width, stride);

    float * buffer = malloc_array(float, 15); //width * width);

    static float const weights[3][3] =
    {
        {1, 2, 1},
        {2, 3, 2},
        {1, 2, 1}
    };

    for (int i = 0; i != size.z; ++ i)
    for (int j = 0; j != size.y; ++ j)
    for (int k = 0; k != size.x; ++ k)
    {
        int target_index = size_index(size, i, j, k);

        for (int l = 0; l != 3; ++ l)
        {
            int count = 0;

            for (int m = -width_2; m <= width_2; ++ m)
            {
                if (j + m < 0 || j + m >= size.y) continue;

                for (int n = -width_2; n <= width_2; ++ n)
                {
                    if (k + n < 0 || k + n > size.x) continue;
                    
                    float value = source_pixels[(target_index + offsets[(width_2 + m) * width + width_2 + n]) * 3 + l];
                    for (int p = 0; p != weights[m + width_2][n + width_2]; ++ p)
                    {
                        buffer[count ++] = value;
                    }

                }
            }

            qsort(buffer, count, sizeof(float), compare);

            target_pixels[target_index * 3 + l] = buffer[median_index];
        }
    }

    free(offsets);
    free(buffer);

    return target;
}

void image_add_procedural(Image * image, float (* proc)(void *), void * data)
{
    Image_Format format = image->format;

    int count = format_to_size(format.format) * size_total(format.size);

    switch (format.type)
    {
        case GL_UNSIGNED_BYTE:
        {
            unsigned char * pixels = (unsigned char *) image->pixels;
            for (int i = 0; i != count; ++ i)
            {
                pixels[i] += (unsigned char) floor(256 * proc(data));
            }
            break;
        }
            
        case GL_UNSIGNED_SHORT:
        {
            unsigned short * pixels = (unsigned short *) image->pixels;
            for (int i = 0; i != count; ++ i)
            {
                pixels[i] += (unsigned short) floor(65536 * proc(data));
            }
            break;
        }

        case GL_FLOAT:
        {
            float * pixels = (float *) image->pixels;
            for (int i = 0; i != count; ++ i)
            {
                pixels[i] += proc(data);
            }
            break;
        }

        case GL_DOUBLE:
        {
            float * pixels = (float *) image->pixels;
            for (int i = 0; i != count; ++ i)
            {
                pixels[i] += proc(data);
            }
            break;
        }
        default:
           error_fail("image_add_procedural: unsupported type");
           break;
    }
}

static float proc_normal(void * data)
{
    float mu    = ((float *) data)[0];
    float sigma = ((float *) data)[1];

    float x = random2();
    float y = random2();
    float z = sqrt(-2.0 * log(x)) * cos(2.0 * M_PI * y);

    return mu + sigma * z;
}

void image_add_noise_normal(Image * image, float mu, float sigma)
{
    float data[] = {mu, sigma};
    image_add_procedural(image, proc_normal, &data);
}

static float proc_uniform(void * data)
{
    float a = ((float *) data)[0];
    float b = ((float *) data)[1];

    return random_range(a, b);
}

void image_add_noise_uniform(Image * image, float a, float b)
{
    float data[] = {a, b};
    image_add_procedural(image, proc_uniform, &data);
}

void image_add_noise(Image * image, float sigma)
{
    Image_Format format = image->format;

    error_check(format.type != GL_FLOAT, "image_add_noise image type must be float");
    error_check(format.format != GL_LUMINANCE && format.format != GL_RGB, "image_add_noise image format must be luminance or rgb");

    int pixel_count = image_format_pixels(format);

    switch (format.format)
    {
        case GL_LUMINANCE:
        {
            float * pixels  = (float *) image->pixels;
            for (int i = 0; i < pixel_count; ++ i)
            {
                pixels[i] += randn() * sigma;
            }
            break;
        }
        case GL_RGB:
        {
            Color * pixels  = (Color *) image->pixels;
            for (int i = 0; i < pixel_count; ++ i)
            {
                pixels[i].r += randn() * sigma;
                pixels[i].g += randn() * sigma;
                pixels[i].b += randn() * sigma;
            }
            break;
        }
    }
}

void image_add_2(Image const * a, Image const * b, Image * c)
{
    Image_Format a_format = a->format;
    Image_Format b_format = b->format;
    Image_Format c_format = c->format;

    error_check(a_format.type != GL_FLOAT, "image_add_2: type must be float");
    error_check(a_format.format != GL_LUMINANCE && a_format.format != GL_RGB, "image_add_2: format must be luminance or rgb");
    error_check(! image_format_equal(a_format, b_format) || ! image_format_equal(a_format, c_format), "image_add_2: image_format must match");

    int pixel_count = image_format_pixels(a_format);

    switch (a_format.format)
    {
        case GL_LUMINANCE:
        {
            float const * a_pixels = (float const *) a->pixels;
            float const * b_pixels = (float const *) b->pixels;
            float       * c_pixels = (float       *) c->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                c_pixels[i] = a_pixels[i] + b_pixels[i];
            }
            break;
        }
        case GL_RGB:
        {
            Color const * a_pixels = (Color const *) a->pixels;
            Color const * b_pixels = (Color const *) b->pixels;
            Color       * c_pixels = (Color       *) c->pixels;

            for (int i = 0; i != pixel_count; ++ i)
            {
                c_pixels[i] = color_add(a_pixels[i], b_pixels[i]);
            }
            break;
        }
    }
}

void image_yuv_to_rgb(Image * image)
{
    error_check(image->format.type != GL_FLOAT, "type must be float");
    error_check(image->format.format != GL_RGB, "format must be rgb");

    int pixel_count = image_format_pixels(image->format);
    Color * pixels = image->pixels;

    for (int i = 0; i != pixel_count; ++ i)
    {
        Color c = pixels[i];
        Color d =
        {
            c.r / sqrt(3) + c.g / sqrt(2) + c.b / sqrt(6),
            c.r / sqrt(3)             - 2 * c.b / sqrt(6),
            c.r / sqrt(3) - c.g / sqrt(2) + c.b / sqrt(6)
        };
        pixels[i] = d;
    }
}

void image_rgb_to_yuv(Image * image)
{   
    printf("%d\n", image->format.format);

    error_check(image->format.type != GL_FLOAT, "type must be float");
    error_check(image->format.format != GL_RGB, "format must be rgb");

    int pixel_count = image_format_pixels(image->format);
    Color * pixels = (Color *) image->pixels;

    for (int i = 0; i != pixel_count; ++ i)
    {   
        Color c = pixels[i];
        Color d = 
        {   
            (c.r +   c.g + c.b) / sqrt(3),
            (c.r         - c.b) / sqrt(2),
            (c.r - 2*c.g + c.b) / sqrt(6)
        };  
        pixels[i] = d;
    }   
} 

