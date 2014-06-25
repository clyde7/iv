#include "box.h"
#include "color.h"
#include "debug.h"
#include "font.h"
#include "geometry.h"
#include "math_.h"
#include "memory.h"
#include "opengl.h"
#include "time_.h"
#include "volume.h"

#define KERNING

#if defined(CYGWIN) || defined(WINDOWS)
static char const * font_path;
#elif defined(DARWIN)
static char const * font_path = "/Library/Fonts";
#elif defined(LINUX)
static char const * font_path = "/usr/share/fonts/truetype";
#endif

Vector const
    ANCHOR_TOP_LEFT      = {-1, +1, 0},
    ANCHOR_TOP_CENTER    = { 0, +1, 0},
    ANCHOR_TOP_RIGHT     = {+1, +1, 0},
    ANCHOR_MIDDLE_LEFT   = {-1,  0, 0},
    ANCHOR_MIDDLE_CENTER = { 0,  0, 0},
    ANCHOR_MIDDLE_RIGHT  = {+1,  0, 0},
    ANCHOR_BOTTOM_LEFT   = {-1, -1, 0},
    ANCHOR_BOTTOM_CENTER = { 0, -1, 0},
    ANCHOR_BOTTOM_RIGHT  = {+1, -1, 0};

int font_debug;

char const * font_map(char const font_name[])
{
#if defined(CYGWIN) || defined(WINDOWS)
    if (streq(font_name, "arial"))       return "arial.ttf";
    if (streq(font_name, "arial black")) return "ariblk.ttf";
    if (streq(font_name, "arial bold"))  return "arialbd.ttf";
    if (streq(font_name, "palatino"))    return "pala.ttf";
#elif defined(LINUX)
//    if (streq(font_name, "arial"))       return "freefont/FreeSans.ttf";
    if (streq(font_name, "arial"))       return "msttcorefonts/arial.ttf";
    if (streq(font_name, "arial black")) return "msttcorefonts/ariblk.ttf";
    if (streq(font_name, "arial bold"))  return "msttcorefonts/arialbd.ttf";
    if (streq(font_name, "palatino"))    return "freefont/FreeSerif.ttf";
#elif defined(DARWIN)
    //if (streq(font_name, "arial"))       return "Arial Unicode.ttf";
    if (streq(font_name, "arial"))       return "Arial.ttf";
    if (streq(font_name, "arial bold"))  return "Arial Bold.ttf";
    if (streq(font_name, "arial black")) return "Arial Black.ttf";
    if (streq(font_name, "big caslon"))  return "BigCaslon.ttf";
    if (streq(font_name, "palatino"))    return "Palatino";
    if (streq(font_name, "Papyrus"))     return "Papyrus.ttc";
    if (streq(font_name, "Calibri"))     return "CALIBRI.TTF";
    if (streq(font_name, "Candara"))     return "CANDARA.TTF";
    if (streq(font_name, "verdana"))     return "Verdana.ttf";
    if (streq(font_name, "verdana bold")) return "Verdana Bold.ttf";
#endif

    return font_name;
}

Font_ * font_create(char const name[], size_t size)
{
    static FT_Library library;
    static int initialized;

    if (! initialized)
    {
        FT_Init_FreeType(&library);
        initialized = 1;
    }

    FT_Face face;
    int error = FT_New_Face(library, name, 0, &face);
    if (error)
    {
        fprintf(stderr, "failed to open font \"%s\"\n", name);
        return NULL;
    }

    FT_Set_Pixel_Sizes(face, 0, size);
    if (font_debug)
        printf("has kerning = %d\n", FT_HAS_KERNING(face) ? 1 : 0);
    
    Font_ * font = malloc_size(Font_);
    font->face = face;
    font->texture = 0;
    font->texture_width = 0;
    font->texture_height = 0;
    return font;
}

void font_destroy(Font_ * font)
{
    FT_Done_Face(font->face);
    free(font);
}

IBox font_bounds(Font_ * font, char const string[])
{
    wchar_t buffer[256];
    mbstowcs(buffer, string, 256);
    return font_bounds_unicode(font, buffer);
}

IBox font_bounds_unicode(Font_ * font, wchar_t const string[])
{
    int i, error;

    FT_Face face = font->face;
    FT_GlyphSlot glyph = face->glyph;
#ifdef KERNING
    int has_kerning = FT_HAS_KERNING(face);
#endif
    int const length = wcslen(string);

    int x, y, x1, y1, x2, y2;
    x = y = x1 = y1 = x2 = y2 = 0;

    // XXX should be infinity
    x1 = 1000;
    y1 = 1000;
#ifdef KERNING
    FT_UInt previous_glyph_index = 0;

    for (i = 0; i != length; ++ i)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, string[i]);
        if (has_kerning)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta);
            x += delta.x >> 6;
        }

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
        if (error)
            continue;

        x1 = imin(x1, x + glyph->bitmap_left);
        x2 = imax(x2, x + glyph->bitmap_left + glyph->bitmap.width);

        y1 = imin(y1, y + glyph->bitmap_top - glyph->bitmap.rows);
        y2 = imax(y2, y + glyph->bitmap_top);

        x += glyph->advance.x >> 6;
        y += glyph->advance.y >> 6;

        previous_glyph_index = glyph_index;
    }
#else
    for (i = 0; i != length; ++ i)
    {
        error = FT_Load_Char(face, string[i], FT_LOAD_RENDER);
        if (error)
            continue;

        x1 = imin(x1, x + glyph->bitmap_left);
        x2 = imax(x2, x + glyph->bitmap_left + glyph->bitmap.width);

        y1 = imin(y1, y + glyph->bitmap_top - glyph->bitmap.rows);
        y2 = imax(y2, y + glyph->bitmap_top);

        x += glyph->advance.x >> 6;
        y += glyph->advance.y >> 6;
    }
#endif

    IBox box = {{x1, y1, 0}, {x2, y2, 0}};
    return box;
}

static Brick font_to_texture_unicode(Font_ * font, wchar_t const string[])
{
    static int width = 0;
    static int height = 0;

    static unsigned char * buffer = NULL;
    int i, j, error;

    FT_Face face = font->face;
    FT_GlyphSlot glyph = face->glyph;
#ifdef KERNING
    int has_kerning = FT_HAS_KERNING(face);
#endif
    int const length = wcslen(string);

    IBox box = font_bounds_unicode(font, string);

// TODO save the old bounds, then patch with 0.
//    float min_x = box.min.x;
//    float min_y = box.min.y;
//    box.min.x = 0;
//    box.min.y = 0;
//
//printf("box min x = %d\n", box.min.x);
    int x1 = 0; //box.min.x;
    int y1 = box.min.y;
    int x2 = box.max.x;
    int y2 = box.max.y;

    int total_width = x2 - x1;
    int total_height = y2 - y1;

    if (total_width > width || total_height > height)
    {
        buffer = realloc_array(unsigned char, buffer, total_width * total_height);
        width = total_width;
        height = total_height;
    }

    int x = 0, y = 0;

    memset(buffer, 0, width * height); /* conservative, but total_width * total_height is to aggressive */

#ifdef KERNING
    FT_UInt previous_glyph_index = 0;
    for (i = 0; i != length; ++ i)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, string[i]);
        if (has_kerning)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta);

            x += delta.x >> 6;
        }

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
        if (error)
            continue;

// XXX test
//x += glyph->bitmap_left;

        FT_Bitmap * bitmap = &glyph->bitmap;
        for (j = 0; j != bitmap->rows; ++ j)
        {
            int k = 0;
            for (k = 0; k != bitmap->width; ++ k)
            {
                unsigned char * target = (unsigned char *) &buffer[(y2 - glyph->bitmap_top + j) * width + x + glyph->bitmap_left + k];
                unsigned char source = bitmap->buffer[j * bitmap->pitch + k];

                if (source > *target)
                    *target = source;
            }
        }

#if 0
        if (font_debug && string[i] == '1')
{
printf("bitmap_width = %d, height = \n", bitmap->width, bitmap->rows);

printf("top = %d, left = %d\n", glyph->bitmap_top, glyph->bitmap_left);
        for (j = 0; j != bitmap->rows; ++ j)
        {
            int k = 0;
            for (k = 0; k != bitmap->width; ++ k)
            {
                //unsigned char * target = (unsigned char *) &buffer[(y2 - glyph->bitmap_top + j) * width + x + glyph->bitmap_left + k];
                unsigned char source = bitmap->buffer[j * bitmap->pitch + k];

                printf("%c", source ? 'X' : '.');
            }
            puts("");
        }
}
#endif

        x += glyph->advance.x >> 6;
        y += glyph->advance.y >> 6;

        previous_glyph_index = glyph_index;
    }
#else
    for (i = 0; i != length; ++ i)
    {
        error = FT_Load_Char(face, string[i], FT_LOAD_RENDER);
        if (error)
            continue;

        FT_Bitmap * bitmap = &glyph->bitmap;
        for (j = 0; j != bitmap->rows; ++ j)
        {
            /* XXX should subtract x1 from x? */
            memcpy(&buffer[(y2 - glyph->bitmap_top + j) * width + x + blyph->bitmap_left], &bitmap->buffer[j * bitmap->pitch], bitmap->width);
        }

        x += glyph->advance.x >> 6;
        y += glyph->advance.y >> 6;
    }
#endif

    if (width > font->texture_width || height > font->texture_height)
    {
        font->texture_width = round_to_power_of_two(width);
        font->texture_height = round_to_power_of_two(height);

        char * clearer = (char *) calloc(font->texture_width * font->texture_height, 1);

        if (font_debug) 
            printf("texture size = %dx%d\n", font->texture_width, font->texture_height);

        if (font->texture == 0)
            glGenTextures(1, &font->texture);

        glBindTexture(GL_TEXTURE_2D, font->texture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font->texture_width, font->texture_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, clearer);

        free(clearer);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

        GLenum filter = GL_LINEAR; /* parameter? */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

        GLenum texture_wrap = GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, font->texture);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, buffer);

    Brick brick =
    {
         font->texture, 0,
         {
//             {0, (float) total_height / font->texture_height, 0},
             {(float) box.min.x / font->texture_width, (float) total_height / font->texture_height, 0},
             {(float) total_width / font->texture_width, 0, 0}
         },
         //{{x1, y1, 0}, {x2, y2, 0}}
         {{0, y1, 0}, {x2 - box.min.x, y2, 0}}
    };
    return brick;
}

static Brick font_to_texture(Font_ * font, char const string[])
{ 
    wchar_t buffer[256];
    mbstowcs(buffer, string, 256);
    return font_to_texture_unicode(font, buffer);
}

/* consider writing a map coordinates helper function */
/* consider begin_text(), end_text() functions */
/* return metrics about width, baseline, and maybe height */

void font_begin(Viewport viewport)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(-1, -1, 0);
    glScalef(2.0 / viewport.width, 2.0 / viewport.height, 1);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
}

void font_end(void)
{
    glDisable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void font_render_exact(Font_ * font, char const string[], Vector raster_position, Vector anchor)
{
    wchar_t buffer[256];
    mbstowcs(buffer, string, 256);
    font_render_exact_unicode(font, buffer, raster_position, anchor);
}

void font_render_exact_unicode(Font_ * font, wchar_t const string[], Vector raster_position, Vector anchor)
{
    Brick brick = font_to_texture_unicode(font, string);
    Box v = brick.box;
    Box t = brick.tex;

    float delta_x = raster_position.x - (anchor.x + 1) * (v.max.x - v.min.x) / 2;
    float delta_y = raster_position.y - (anchor.y + 1) * (v.max.y - v.min.y) / 2;

// XXX test
#if 0
if ((((int) (v.max.x - v.min.x)) % 2) == 0)
    delta_x = delta_x + 0.5;
#else
//    delta_x = floor(delta_x);
//    delta_y = floor(delta_y);

if ((((int) (v.max.x - v.min.x)) % 2) == 1)
    delta_x = delta_x - 0.5;
if ((((int) (v.max.y - v.min.y)) % 2) == 1)
    delta_y = delta_y - 0.5;
#endif

    v.min.x += delta_x;
    v.max.x += delta_x;
    v.min.y += delta_y;
    v.max.y += delta_y;

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(t.min.x, t.min.y); glVertex2f(v.min.x, v.min.y);
    glTexCoord2f(t.min.x, t.max.y); glVertex2f(v.min.x, v.max.y);
    glTexCoord2f(t.max.x, t.min.y); glVertex2f(v.max.x, v.min.y);
    glTexCoord2f(t.max.x, t.max.y); glVertex2f(v.max.x, v.max.y);
    glEnd();
}

void font_render_exact_shadow(Font_ * font, char const string[], Vector raster_position, Vector anchor)
{
    static Vector offset = {+2, -2, 0};
    color_apply(BLACK);
    font_render_exact(font, string, vector_add(raster_position, offset), anchor);

    color_apply(WHITE);
    font_render_exact(font, string, raster_position, anchor);
}

void font_render(Font_ * font, char const string[], Matrix matrix, Viewport viewport, Vector position, Vector anchor)
{
    Vector projected_position = matrix_mul(matrix, position);
    Vector raster_position =
    {
        (projected_position.x + 1) * viewport.width / 2.0,
        (projected_position.y + 1) * viewport.height / 2.0,
        0 // projected_position.z
    };

    font_render_exact(font, string, raster_position, anchor);
}

Font_ * font_open(char const name[], size_t size)
{
    char * canonic = strdup(name);
    string_lower(canonic);
    char const * file_name = font_map(canonic);
    free(canonic);

    char buffer[256];
    buffer[0] = '\0';

    if (! font_path)
        font_path = getenv("FONT_PATH");

    if (! font_path)
    {
#if defined(DARWIN)
        sprintf(buffer, "/Users/cknaus/Library/Fonts/");
#elif defined(CYGWIN) || defined(WINDOWS)
        sprintf(buffer, "%s/Fonts/", getenv("WINDIR"));
#else
        fprintf(stderr, "please specify $FONT_PATH\n");
#endif
    }
    else
    {
        if (strlen(font_path) != 0)
            sprintf(buffer, "%s/", font_path);
    }

    strcat(buffer, file_name);

    printf("loading font \"%s\"", FILE_NAME(buffer));
    Time_ begin = time_current();
    Font_ * font = font_create(buffer, size);
    font
        ? printf(" (%.2f seconds)\n", time_duration(begin))
        : printf(" failed!\n");

    return font;
}

void font_render_text_box(Font_ * font, char const string[], Vector position, Vector anchor)
{
    IBox ibox = font_bounds(font, string);
    Box box = {{ibox.min.x, ibox.min.y, 0}, {ibox.max.x, ibox.max.y, 0}};

    Vector delta =
    {
        position.x - (anchor.x + 1) * (box.max.x - box.min.x) / 2,
        position.y - (anchor.y + 1) * (box.max.y - box.min.y) / 2,
        0
    };

    box = box_translate(box, delta);
    box = box_expand(box, 8);

    glDisable(GL_TEXTURE_2D);

    color_apply(RED);
    fill_rect_rounded(box, 6.0, 4);

    glLineWidth(2.0);
    color_apply(WHITE);
    draw_rect_rounded(box, 6.0, 4);

    glEnable(GL_TEXTURE_2D);

    font_render_exact(font, string, position, anchor);
}

void font_render_text_circle(Font_ * font, char const string[], Vector position, Vector anchor)
{
    IBox ibox = font_bounds(font, "99");
    Box box = {{ibox.min.x, ibox.min.y, 0}, {ibox.max.x, ibox.max.y, 0}};

    Vector delta =
    {
        position.x - (anchor.x + 1) * (box.max.x - box.min.x) / 2,
        position.y - (anchor.y + 1) * (box.max.y - box.min.y) / 2,
        0
    };

    box = box_translate(box, delta);
    box = box_expand(box, 3);

    Vector center, corner;
    box_bounding_sphere(box, &center, &corner);
    float radius = vector_length(corner);

    glDisable(GL_TEXTURE_2D);

    color_apply(RED);
    fill_circle(center, radius, 32);

    glLineWidth(2.0);
    color_apply(WHITE);
    draw_circle(center, radius, 32);

    glEnable(GL_TEXTURE_2D);

    font_render_exact(font, string, position, anchor);
}

