#ifndef VOLUME_H
#define VOLUME_H

#include "box.h"
#include "image.h"
#include "matrix.h"
#include "opengl.h"

typedef struct {GLuint texture, level; Box tex, box;} Brick;

void download_palette(Image const * palette);

GLuint volume_create_texture(Image const *, int gradients);
GLuint volume_create_geometry(unsigned num_slices, unsigned resolution_x, unsigned resolution_y);
void   volume_render_begin(void);
void   volume_render_end(void);
void   volume_render_brick(GLuint geometry, Matrix const * inverse_modelview, Brick const *);
Brick * volume_create_brick(Image const *, Vector ratio);
void   volume_destroy_brick(Brick *);

#endif
