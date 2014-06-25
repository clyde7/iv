#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "math_.h"
#include "memory.h"
#include "opengl.h"
#include "texture.h"

static GLenum dimension_to_target(unsigned dimension)
{
    switch (dimension)
    {
        case 1 : return GL_TEXTURE_1D;
        case 2 : return GL_TEXTURE_2D;
        case 3 : return GL_TEXTURE_3D;
    }

    return GL_TEXTURE_2D;
}

Color texture_checker(Vector v)
{
    int n =
        (int) floor(v.x) +
        (int) floor(v.y) +
        (int) floor(v.z);

    if (n < 0)
        n = 1 - n;

    return n % 2 ? WHITE : BLACK;
}

void texture_download(Image const * image)
{
    Image_Format format = image->format;
    Size size = format.size;
    GLenum internal_format = format.format;
    unsigned dimension = image_format_dimension(format);

    switch (dimension)
    {
        case 1: glTexImage1D(GL_TEXTURE_1D, 0, internal_format, size.x,                 0, format.format, format.type, image->pixels); break;
        case 2: glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.x, size.y,         0, format.format, format.type, image->pixels); break;
        case 3: glTexImage3D(GL_TEXTURE_3D, 0, internal_format, size.x, size.y, size.z, 0, format.format, format.type, image->pixels); break;
    }
}

void texture_download_target(Image const * image, GLenum target)
{
    Image_Format format = image->format;
    Size size = format.size;
    GLenum internal_format = format.format;
    unsigned dimension = image_format_dimension(format);

    switch (dimension)
    {
        case 1: glTexImage1D(target, 0, internal_format, size.x,                 0, format.format, format.type, image->pixels); break;
        case 2: glTexImage2D(target, 0, internal_format, size.x, size.y,         0, format.format, format.type, image->pixels); break;
        case 3: glTexImage3D(target, 0, internal_format, size.x, size.y, size.z, 0, format.format, format.type, image->pixels); break;
    }
}

void texture_download_layer(Image const * image, int layer)
{
    Image_Format format = image->format;
    Size size = format.size;
    GLenum internal_format = format.format;
    unsigned dimension = image_format_dimension(format);

    error_check(dimension != 2 && dimension != 3, "dimension must be 2 or 3");

    GLubyte const * pixels = (GLubyte const *) image->pixels;
    int stride_z = size.x * size.y * image_format_pixel_size(format);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.x, size.y, 0, format.format, format.type, &pixels[layer * stride_z]);
}

float luminance_overcast_sky(Vector omega)
{
    float r = sqrt(omega.x * omega.x + omega.y + omega.y);

    return (1 + 2*r) / 3;
}

float luminance_clear_sky(Vector omega, Vector sun)
{
    static float const a = 0.91, b = 0.45, c = 0.32;
    float r_o = sqrt(omega.x * omega.x + omega.y + omega.y);
    float r_s = sqrt(sun.x * sun.x + sun.y + sun.y);
    float theta = asin(r_o); /* angle from zenith */
    float S = asin(r_s); /* angle from zenith */
    float gamma = acos(dot(omega, sun));

    return
        (a + 10*exp(-3*gamma) + b*cos(gamma)*cos(gamma)) * (1 - exp(-c/cos(theta))) /
        (a + 10*exp(-3*S) + b*cos(S)*cos(S)) * (1 - exp(-c));
}

float luminance_clear_sky_simple(Vector omega, Vector sun)
{
    static float const a = 0.526, b = 1.5, c = 0.8;
    float r_s = sqrt(sun.x * sun.x + sun.y + sun.y);
    float S = asin(r_s); /* angle from zenith */
    float gamma = acos(dot(omega, sun));

    return
        ((a + 5*exp(-b*gamma)) * (1 - exp(-c/cos(gamma)))) /
        ((a + 5*exp(-b*S))     * (1 - exp(-c)));
}

float luminance_head(Vector omega, float cut_off_angle)
{
    float radius = sqrt(omega.x * omega.x + omega.y + omega.y);
    float theta = asin(radius);

    return theta < cut_off_angle ? 1 : 0;
}

Brick * brick_from_image(Image const * image)
{
    Image_Format format = image->format;
    unsigned dimension = image_format_dimension(format);
    GLenum target = dimension_to_target(dimension);
    GLenum internal_format = format.format;

    int width  = round_to_power_of_two(format.size.x);
    int height = round_to_power_of_two(format.size.y);
    int depth  = round_to_power_of_two(format.size.z);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(target, texture);

    switch (dimension)
    {
        case 1: glTexImage1D(target, 0, internal_format, width,                0, format.format, format.type, image->pixels); break;
        case 2: glTexImage2D(target, 0, internal_format, width, height,        0, format.format, format.type, image->pixels); break;
        case 3: glTexImage3D(target, 0, internal_format, width, height, depth, 0, format.format, format.type, image->pixels); break;
    }

    GLenum filter = GL_LINEAR; /* parameter? */
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);

//    GLenum texture_wrap = GL_CLAMP_TO_EDGE;
//    glTexParameteri(target, GL_TEXTURE_WRAP_S, texture_wrap);
//    glTexParameteri(target, GL_TEXTURE_WRAP_T, texture_wrap);
//    glTexParameteri(target, GL_TEXTURE_WRAP_R, texture_wrap);

    Vector tex_max =
    {
        (float) format.size.x / width,
        (float) format.size.y / height,
        (float) format.size.z / depth
    };

    Brick * brick = malloc_size(Brick);
    brick->texture = texture;
    brick->level = 0;
    brick->tex.min = ORIGIN;
    brick->tex.max = tex_max;
    brick->box.min = ORIGIN;
    brick->box.max = vector(format.size.x, format.size.y, format.size.z);

    return brick;
}
