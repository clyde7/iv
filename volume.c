/* Copyright Claude Knaus. All rights reserved. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "math_.h"
#include "memory.h"
#include "matrix.h"
#include "volume.h"

static unsigned char * create_gradients(Image const * volume)
{
    Image_Format format = volume->format;
    unsigned const sx = 1;
    unsigned const sy = format.size.x;
    unsigned const sz = format.size.x * format.size.y;
    int i, j, k, index = 0;
    signed dx, dy, dz;
    unsigned value;
    float scale;

    /* XXX other formats? */
    unsigned char const * input = (unsigned char *) volume->pixels;
    unsigned char * data = (unsigned char *) malloc(size_total(format.size) * 4 * sizeof(unsigned char));
    unsigned char * output = data;

    for (i = 0; i != format.size.z; ++ i)
    for (j = 0; j != format.size.y; ++ j)
    for (k = 0; k != format.size.x; ++ k)
    {
        dx = (k == format.size.x - 1 ? 0 : input[index + sx]) - (k == 0 ? 0 : input[index - sx]);
        dy = (j == format.size.y - 1 ? 0 : input[index + sy]) - (j == 0 ? 0 : input[index - sy]);
        dz = (i == format.size.z - 1 ? 0 : input[index + sz]) - (i == 0 ? 0 : input[index - sz]);
        value = input[index];

        scale = 127.0 / sqrt((float) (dx * dx + dy * dy + dz * dz));

        *output++ = 127 + dx * scale;
        *output++ = 127 + dy * scale;
        *output++ = 127 + dz * scale;
        *output++ = value;

        ++index;
    }

    return data;
}

void download_palette(Image const * palette)
{
    Image_Format format = palette->format;
    int const map_size = format.size.x;

    /* XXX relax for other format and types */
    assert(format.format == GL_RGBA);
    assert(format.type == GL_UNSIGNED_BYTE);
    assert(format.size.y == 1);
    assert(format.size.z == 1);

    unsigned char * pixels = (unsigned char *) palette->pixels;

    float * map[4] =
    {
		(float *) malloc(map_size * sizeof(GLubyte)),
		(float *) malloc(map_size * sizeof(GLubyte)),
		(float *) malloc(map_size * sizeof(GLubyte)),
		(float *) malloc(map_size * sizeof(GLubyte))
    };

    int i;
    for (i = 0; i != map_size; ++ i)
    {
        map[0][i] = pixels[i * 4 + 0] / 255.0;
        map[1][i] = pixels[i * 4 + 1] / 255.0;
        map[2][i] = pixels[i * 4 + 2] / 255.0;
        map[3][i] = pixels[i * 4 + 3] / 255.0;
    }

    glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, map_size, map[0]);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, map_size, map[1]);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, map_size, map[2]);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, map_size, map[3]);

    free(map[0]);
    free(map[1]);
    free(map[2]);
    free(map[3]);
}

GLuint volume_create_texture(Image const * volume, int const gradients)
{
    Image_Format format = volume->format;
    Image const * palette = volume->palette;

    GLenum const type = format.type;
    GLenum internal_format, external_format;
    unsigned char * data;

    if (gradients && type == GL_UNSIGNED_BYTE)
    {
        data = create_gradients(volume);

        external_format = GL_RGBA;
        internal_format = GL_RGBA;
    }
    else if (palette)
    {
        download_palette(palette);

        data = (unsigned char *) volume->pixels;
        external_format = GL_COLOR_INDEX;
        internal_format = palette->format.format; /* GL_RGBA, GL_LUMINANCE_ALPHA, or GL_INTENSITY */
    }
    else
    {
        data = (unsigned char *) volume->pixels;
        external_format = GL_LUMINANCE;
        internal_format = GL_INTENSITY;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef MIPMAPS
    gluBuild3DMipmaps(GL_TEXTURE_3D, internal_format,
        format.size.x, format.size.y, format.size.z, external_format, type,
        data);
#else
    glTexImage3D(GL_TEXTURE_3D, 0, internal_format,
        format.size.x, format.size.y, format.size.z, 0, external_format, type,
        data);
#endif

    GLenum const error = glGetError();
    error_check(error, "failed to download texture");

    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

//    static GLint const texture_wrap = GL_CLAMP_TO_EDGE;
//    static GLint const texture_wrap = GL_MIRRORED_REPEAT;
    static GLint const texture_wrap = GL_REPEAT;

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, texture_wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, texture_wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, texture_wrap);

    return texture;
}

void volume_render_begin(void)
{
    glEnable(GL_TEXTURE_3D);

    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    glEnable(GL_CLIP_PLANE3);
    glEnable(GL_CLIP_PLANE4);
    glEnable(GL_CLIP_PLANE5);
}

void volume_render_end(void)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);

    glDisable(GL_TEXTURE_3D);
}

void volume_render_brick(GLuint const geometry, Matrix const * inverse_modelview, Brick const * const brick)
{
    Vector size = box_size(brick->box);
    Vector tex_size = box_size(brick->tex);
    GLfloat const texture_matrix[4][4] =
    {
        {tex_size.x / size.x, 0, 0, 0},
        {0, tex_size.y / size.y, 0, 0},
        {0, 0, tex_size.z / size.z, 0},
        {brick->tex.min.x, brick->tex.min.y, brick->tex.min.z, 1}
    };
    GLdouble const clip_planes[6][4] =
    {
        { 1, 0, 0, -brick->box.min.x},
        {-1, 0, 0,  brick->box.max.x},
        { 0, 1, 0, -brick->box.min.y},
        { 0,-1, 0,  brick->box.max.y},
        { 0, 0, 1, -brick->box.min.z},
        { 0, 0,-1,  brick->box.max.z}
    };

    glBindTexture(GL_TEXTURE_3D, brick->texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, brick->level);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, brick->level);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(&texture_matrix[0][0]);
    glTranslatef(-brick->box.min.x, -brick->box.min.y, -brick->box.min.z); // move into texture_matrix
    glMultMatrixf((GLfloat const *) inverse_modelview);
    glMatrixMode(GL_MODELVIEW);

    glClipPlane(GL_CLIP_PLANE0, clip_planes[0]);
    glClipPlane(GL_CLIP_PLANE1, clip_planes[1]);
    glClipPlane(GL_CLIP_PLANE2, clip_planes[2]);
    glClipPlane(GL_CLIP_PLANE3, clip_planes[3]);
    glClipPlane(GL_CLIP_PLANE4, clip_planes[4]);
    glClipPlane(GL_CLIP_PLANE5, clip_planes[5]);

    glCallList(geometry);
}   

static void draw_flat(unsigned const num_slices, GLfloat const left, GLfloat const right, GLfloat const bottom, GLfloat const top, GLfloat const near_, GLfloat const far_)
{
    unsigned i;

    glBegin(GL_QUADS);
    for (i = 0; i != num_slices; ++ i)
    {
        GLfloat const t = (i + 0.5) / num_slices;
        GLfloat const z = - (1 - t) * far_ - t * near_;
        GLfloat const x1 = -z * left / near_;
        GLfloat const x2 = -z * right / near_;
        GLfloat const y1 = -z * bottom / near_;
        GLfloat const y2 = -z * top / near_;

        glVertex3f(x1, y1, z);
        glVertex3f(x2, y1, z);
        glVertex3f(x2, y2, z);
        glVertex3f(x1, y2, z);
    }
    glEnd();
}

static void draw_spherical(unsigned num_slices, unsigned const resolution_x, unsigned const resolution_y, GLfloat const left, GLfloat const right, GLfloat const bottom, GLfloat const top, GLfloat const near_, GLfloat const far_)
{
    unsigned i, j, k;

    float const phi_left = atan2(left, near_);
    float const phi_right = atan2(right, near_);
    float const tau_bottom = atan2(bottom, near_);
    float const tau_top = atan2(top, near_);

    float const max_h = fmax(fabs(left * far_ / near_), fabs(right * far_ / near_));
    float const max_v = fmax(fabs(bottom * far_ / near_), fabs(top * far_ / near_));

    float const radius_near = near_;
    float const radius_far = sqrt(max_h * max_h + max_v * max_v + far_ * far_);

    num_slices *= (radius_far - radius_near) / (far_ - near_);

    for (i = 0; i != num_slices; ++ i)
    {
        float const t = (float) i / num_slices;
        float const radius = radius_near * t + radius_far * (1 - t);

        for (k = 0; k != resolution_y; ++ k)
        {
            float const v1 = (float) k / resolution_y;
            float const v2 = (float) (k + 1) / resolution_y;
            float const tau1 = tau_bottom * (1 - v1) + tau_top * v1;
            float const tau2 = tau_bottom * (1 - v2) + tau_top * v2;
            float const cos_tau1 = cos(tau1);
            float const cos_tau2 = cos(tau2);
            float const sin_tau1 = sin(tau1);
            float const sin_tau2 = sin(tau2);

            glBegin(GL_TRIANGLE_STRIP);
            for (j = 0; j <= resolution_x; ++ j)
            {
                float const u = (float) j / resolution_x;
                float const phi = phi_left * (1 - u) + phi_right * u + M_PI;
                float const cos_phi = cos(phi);
                float const sin_phi = sin(phi);

                GLfloat const x1 = radius * sin_phi * cos_tau1;
                GLfloat const z1 = radius * cos_phi * cos_tau1;
                GLfloat const y1 = radius * sin_tau1;

                GLfloat const x2 = radius * sin_phi * cos_tau2;
                GLfloat const z2 = radius * cos_phi * cos_tau2;
                GLfloat const y2 = radius * sin_tau2;

                glVertex3f(x1, y1, z1);
                glVertex3f(x2, y2, z2);
            }
            glEnd();
        }
    }
}

/* TexGen is ignored when using shaders, but there is no point in removing the code. */
GLuint volume_create_geometry(unsigned num_slices, unsigned const resolution_x, unsigned const resolution_y)
{
    static GLfloat const xPlane[] = {1, 0, 0, 0};
    static GLfloat const yPlane[] = {0, 1, 0, 0};
    static GLfloat const zPlane[] = {0, 0, 1, 0};

    Matrix projection_matrix;
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *) &projection_matrix);
    Projection projection = matrix_invert_projection(projection_matrix);

    assert(projection.type == MATRIX_FRUSTUM); 

    GLuint const list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_GEN_S);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_S, GL_EYE_PLANE, xPlane);

    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_T, GL_EYE_PLANE, yPlane);

    glEnable(GL_TEXTURE_GEN_R);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_R, GL_EYE_PLANE, zPlane);

    // TODO change draw_flat, draw_spherical to use Projection
    if (resolution_x == 0 && resolution_y == 0)
        draw_flat(num_slices, projection.left, projection.right, projection.bottom, projection.top, projection.near_, projection.far_);
    else
        draw_spherical(num_slices, resolution_x, resolution_y, projection.left, projection.right, projection.bottom, projection.top, projection.near_, projection.far_);

    glPopMatrix();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);

    glEndList();

    return list;
}

Brick * volume_create_brick(Image const * volume, Vector ratio)
{
    Image_Format format = volume->format;
    float const size_x = (format.size.x - 1) * ratio.x;
    float const size_y = (format.size.y - 1) * ratio.y;
    float const size_z = (format.size.z - 1) * ratio.z;
    float const max_size = fmax(fmax(size_x, size_y), size_z);

    Brick * brick = malloc_size(Brick);
    brick->texture = 0;
    brick->level = 0;
    brick->tex.min.x = 0.5 / format.size.x;
    brick->tex.min.y = 0.5 / format.size.y;
    brick->tex.min.z = 0.5 / format.size.z;
    brick->tex.max.x = 1 - brick->tex.min.x;
    brick->tex.max.y = 1 - brick->tex.min.y;
    brick->tex.max.z = 1 - brick->tex.min.z;
    brick->box.min = ORIGIN;
    brick->box.max.x = size_x / max_size;
    brick->box.max.y = size_y / max_size;
    brick->box.max.z = size_z / max_size;

    printf("box = \n");
    box_print(brick->box);
    printf("tex = \n");
    box_print(brick->tex);

    return brick;
}

void volume_destroy_brick(Brick * brick)
{
    glDeleteTextures(1, &brick->texture);
    free(brick);
}

