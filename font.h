#ifndef FONT_H
#define FONT_H

#include <stdlib.h>
#include <ft2build.h>
#ifdef LINUX
#include <freetype/freetype.h>
#endif
#ifdef DARWIN
#include <freetype.h>
#endif
#include <wchar.h>

#include "matrix.h"
#include "opengl.h"
#include "size.h"
#include "vector.h"
#include "viewport.h"

typedef struct
{
    FT_Face face;
    GLuint texture;
    int texture_width, texture_height; // TODO use Size
}
Font_;

typedef struct {Size min, max;} IBox;

extern Vector const
    ANCHOR_TOP_LEFT,
    ANCHOR_TOP_CENTER,
    ANCHOR_TOP_RIGHT,
    ANCHOR_MIDDLE_LEFT,
    ANCHOR_MIDDLE_CENTER,
    ANCHOR_MIDDLE_RIGHT,
    ANCHOR_BOTTOM_LEFT,
    ANCHOR_BOTTOM_CENTER,
    ANCHOR_BOTTOM_RIGHT;

extern int font_debug;

Font_ * font_create(char const name[], size_t);
Font_ * font_open(char const name[], size_t);
void   font_destroy(Font_ *);
void   font_begin(Viewport);
void   font_end(void);
void   font_render(Font_ *, char const string[], Matrix, Viewport, Vector position, Vector anchor);
void   font_render_exact(Font_ *, char const string[], Vector raster_position, Vector anchor);
void   font_render_exact_unicode(Font_ *, wchar_t const string[], Vector raster_position, Vector anchor);
void   font_render_exact_shadow(Font_ *, char const string[], Vector raster_position, Vector anchor);
IBox   font_bounds(Font_ *, char const string[]);
IBox   font_bounds_unicode(Font_ *, wchar_t const string[]);

void font_render_text_box(Font_ *, char const string[], Vector position, Vector anchor);
void font_render_text_circle(Font_ *, char const string[], Vector position, Vector anchor);

#endif
