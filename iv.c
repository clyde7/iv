#include <float.h>
#include <stdlib.h>
#include <stdio.h>

#include "action.h"
#include "color.h"
#include "error.h"
#include "font.h"
#include "file_image.h"
#include "geometry.h"
#include "glut.h"
#include "half.h"
#include "image.h"
#include "image_process.h"
#include "list.h"
#include "math_.h"
#include "memory.h"
#include "palette.h"
#include "pixel_map.h"
#include "pixel_transfer.h"
#include "print.h"
#include "string.h"
#include "time_.h"
#include "utils.h"
#include "variable.h"
#include "viewport.h"

static int verbose, fullscreen;
static Viewport viewport;
static Color background;
static Vector translation, delta_translation;
static Matrix forward_matrix, backward_matrix;

static Font_ * font;
static char title[256], image_size[256], string_buffer[256];
static Vector mouse_position, picked_position, grabbed_position;
static int mouse_entered;
static enum {NORMAL, GRABBED, ZOOMING} mode;
static Box zoom_box;
static float const zoom_factor = 1.1;
static int precision = 2;

static List names, boxes;
static int name_index, layer;
static char const ** layer_names;

static Image * source_image, * download_image, * histogram;
static Property * properties;
static unsigned property_count;

static enum {CHANNEL_ALL, CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE} channels;
static float scale = 1.0, contrast = 1.0, gamma_value = 1.0;
static int filter, play, false_colors;
static int delay = 2000;
static int dirty_texture;

static Variable_Extension const names_extension = {&names, name_parser,  NULL};
static Variable_Extension const boxes_extension = {NULL, box_parse,    NULL};
static Variable_Extension const color_extension = {NULL, color_parser, color_printer};
static Variable const variables[] =
{
    {NULL,         'h', NIL, "help",         "-h",  "usage",             NULL},
    {&verbose,     'b', NIL, "verbose",      "-v",  "verbose",           NULL},
    {&background,  'M', NIL, "background",   "-bg", "background color",  &color_extension},
    {&fullscreen,  'b', NIL, "fullscreen",   "-F",  "fullscreen window", NULL},
    {&false_colors,'b', NIL, "false_colors", "-fc", "false colors for luminance",   NULL},
    {&gamma_value, 'f', NIL, "gamma",        "-g",  "gamma value",       NULL},
    {&contrast,    'f', NIL, "contrast",     "-c",  "contrast",          NULL},
    {&scale,       'f', NIL, "scale",        "-s",  "scale",             NULL},
    {&filter,      'b', NIL, "filter",       "-f",  "filter",            NULL},
    {&play,        'b', NIL, "play",         "-p",  "play slideshow",    NULL},
    {&delay,       'd', NIL, "delay",        "-d",  "delay",             NULL},
    {&boxes,       'M', NIL, "highlight",    "-hl", "highlight region",  &boxes_extension},
    {&precision,   'd', NIL, "precision",    "-pr", "precision",         NULL},
    {&names,       's', NIL, NULL,           NULL,  "images",            &names_extension},
};
static int const variable_count = array_count(variables);

#if 0
static void draw_spherical_projection(int nx, int ny)
{
    color_apply(WHITE);

    glEnable(GL_TEXTURE_2D);

    for (int i = 0; i < ny; ++ i)
    {
        float t_1 = (float) i / ny;
        float t_2 = (float) (i + 1) / ny;

        glBegin(GL_TRIANGLE_STRIP);

        for (int j = 0; j <= nx; ++ j)
        {
            float s = (float) j / nx;

            Vector v_1 = map_stereographic(vector(s, t_1, 0));
            Vector v_2 = map_stereographic(vector(s, t_2, 0));

            glTexCoord2f(s, t_1); glVertex2f(v_1.x, v_1.y);
            glTexCoord2f(s, t_2); glVertex2f(v_2.x, v_2.y);
        }

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}
#endif

static Image * load_image(char const name[])
{
    image_destroy(source_image);
    source_image = image_open(name);
    error_check_arg(! source_image, "failed to open file \"%s\"", name);

    if (verbose)
        printf("name = \"%s\"\n", name);

//    property_destroy(properties, property_count);
    properties = image_properties(name, &property_count);
    property_sort(properties, property_count);

    if (string_ends_with(name, ".exr"))
    {
//        property_destroy(properties, property_count);
        properties = exr_image_properties(name, &property_count);
        property_sort(properties, property_count);
//        property_print(properties, property_count);

        int layer_count = source_image->format.size.z;

        free(layer_names);
        layer_names = malloc_array(char const *, layer_count);

        for (int i = 0; i != layer_count; ++ i)
        {
            char layer_key[256];
            sprintf(layer_key, "layer_name.%d", i);

            char const * value = property_find(properties, property_count, layer_key);
            layer_names[i] = value ? strdup(value) : NULL;
        }
    }

    image_destroy(histogram);
    if (source_image->format.type == GL_UNSIGNED_BYTE ||
        source_image->format.type == GL_UNSIGNED_SHORT ||
        source_image->format.type == GL_FLOAT)
        histogram = image_histogram(source_image, 0);
    else
        histogram = NULL;

    if (source_image->format.format == GL_LUMINANCE && source_image->format.type == GL_UNSIGNED_SHORT)
    {
        int zeros = image_count_zeros(source_image);
        int value = image_sum_values(source_image);

        int pixel_count = size_volume(source_image->format.size);

        printf("zeros = %d (%.1f%%)\n", zeros, (float) zeros / pixel_count);
        printf("total samples = %d\n", zeros * 1024 + value);
    }

    Image * float_image = image_retype(source_image, GL_FLOAT);

    // hack to add alpha channel for comparison
    if (float_image->format.format == GL_RGB)
    {
        Image_Format format = float_image->format;
        format.format = GL_RGBA;

        Image * tmp_image = image_new(format);
        image_add_alpha(float_image, tmp_image);

        image_destroy(float_image);
        float_image = tmp_image;
    }

    // XXX hack
//    image_scale(float_image, 65535.0 / 1024.0);

    Color4 const * pixels = (Color4 const *) float_image->pixels;
    if (verbose)
    for (int i = 0; i != boxes.count; ++ i)
    {
        Box box = * (Box *) boxes.entries[i];
        int ii = box.min.z;
        int jj = box.min.y;
        int kk = box.min.x;

        int index = size_index(float_image->format.size, ii, jj, kk);
        Color4 c = pixels[index];

        // TODO mean, var
        printf("%d: ", i); color_print(c.c); puts("");
    }

    return float_image;
}

static void update_labels(void)
{
    char const * name = (char const *) names.entries[name_index];

    if (layer_names)
        sprintf(title, "%s -- %s", name, layer_names[layer]);
    else
        sprintf(title, "%s", name);

    sprintf(image_size, "%dx%d", download_image->format.size.x, download_image->format.size.y);

    if (glutGetWindow())
        glutSetWindowTitle(title);
}

static void update_image(void)
{
    image_destroy(download_image);

    char const * name = (char const *) names.entries[name_index];
    download_image = load_image(name);

    update_labels();
}

static Vector pick(Vector position)
{
    return matrix_mul(backward_matrix, position);
}

static void zoom(float factor)
{
    Vector handle = picked_position;
    Vector t2 = vector_scale(handle, 1 - factor);
    translation = vector_add(translation, vector_scale(t2, scale));
    scale *= factor;
}

static void sprintf_color_value(char buffer[], float value)
{
    char format[256];

    if (contrast == 1)
    {
        sprintf(format, "%%.%df", precision);
        //sprintf(buffer, "%.2f", value);
        sprintf(buffer, format, value);
    }
    else
    {
        sprintf(format, "%%.%df (%%.%df)", precision, precision);
        //sprintf(buffer, "%.2f (%.2f)", value * contrast, value);
        sprintf(buffer, format, value * contrast, value);
    }
}

static void draw_pixel_values(Vector position, Size pixel_coordinates, Color sample)
{
    color_apply(CYAN);
    sprintf(string_buffer, "%d, %d", pixel_coordinates.x, pixel_coordinates.y);
    font_render_exact(font, string_buffer,  vector_add(position, vector(10, -10, 0)), ANCHOR_TOP_LEFT);

    color_apply(RED);
    sprintf_color_value(string_buffer, sample.r);
    font_render_exact(font, string_buffer, vector_add(position, vector(10, -10 -18, 0)), ANCHOR_TOP_LEFT);

    color_apply(GREEN);
    sprintf_color_value(string_buffer, sample.g);
    font_render_exact(font, string_buffer, vector_add(position, vector(10, -10 -36, 0)), ANCHOR_TOP_LEFT);

    color_apply(BLUE);
    sprintf_color_value(string_buffer, sample.b);
    font_render_exact(font, string_buffer, vector_add(position, vector(10, -10 -54, 0)), ANCHOR_TOP_LEFT);
}

static void draw_pixel_values_index(Vector position, Size pixel_coordinates, unsigned short value)
{
    color_apply(CYAN);
    sprintf(string_buffer, "%d, %d", pixel_coordinates.x, pixel_coordinates.y);
    font_render_exact(font, string_buffer,  vector_add(position, vector(10, -10, 0)), ANCHOR_TOP_LEFT);

    color_apply(WHITE);
    sprintf(string_buffer, "%d", value);
    font_render_exact(font, string_buffer, vector_add(position, vector(10, -10 -18, 0)), ANCHOR_TOP_LEFT);
}

static void font_render_local(char const buffer[], Vector position, Vector anchor)
{
    font_render_exact(font, buffer, matrix_mul(forward_matrix, position), anchor);
} 

static void draw_pixel_content(void)
{
    Box box = MIN_BOX;
    box = box_add(box, pick(ORIGIN));

    box = box_add(box, pick(vector(viewport.width, 0, 0)));
    box = box_add(box, pick(vector(0, viewport.height, 0)));
    box = box_add(box, pick(vector(viewport.width, viewport.height, 0)));

    box.min = vector_floor(box.min);
    box.max = vector_ceil(box.max);

    box.min = vector_max(box.min, ORIGIN);
    box.max = vector_min(box.max, vector(download_image->format.size.x, download_image->format.size.y, 0));

    Size min = {(int) box.min.x, (int) box.min.y, 0};
    Size max = {(int) box.max.x, (int) box.max.y, 0};

    font_begin(viewport);

    for (int i = min.y; i != max.y; ++ i)
    for (int j = min.x; j != max.x; ++ j)
    {
        Size pixel_coordinates = {j, i, 0};

        Vector sample_position = {j, i, 0};
        sample_position.z = 0;

        Vector position = matrix_mul(forward_matrix, sample_position);
        position.y += scale;

        sample_position.z = layer;

        if (source_image->format.format == GL_LUMINANCE && source_image->format.type == GL_UNSIGNED_SHORT)
        {
            unsigned short color_index = image_sample_index(source_image, sample_position, BORDER_BLACK);
            draw_pixel_values_index(position, pixel_coordinates, color_index);
        }
        else
        {
            Color pixel_color = image_sample(download_image, sample_position, BORDER_BLACK);
            draw_pixel_values(position, pixel_coordinates, pixel_color);
        }
    }

    font_end();
}

static Color histogram_color(Image_Format format, int channel)
{
    switch (format.format)
    {
        default:
        case GL_LUMINANCE:       return WHITE;
        case GL_LUMINANCE_ALPHA: return channel == 0 ? WHITE : MAGENTA;
        case GL_RGB:             switch (channel) {default: case 0: return RED; case 1: return GREEN; case 2: return BLUE;}
        case GL_RGBA:            switch (channel) {default: case 0: return RED; case 1: return GREEN; case 2: return BLUE; case 3: return MAGENTA;}
    }
}

void draw_histogram(Image const * image)
{
    Image_Format format = image->format;
    int bin_count = format.size.x;
    int channel_count = format_to_size(format.format);
    float const * values = (float const *) image->pixels;

    for (int i = 0; i != channel_count; ++ i)
    {
        Color color = histogram_color(format, i);
        color_apply(color);

        glBegin(GL_LINE_STRIP);
        for (int j = 0; j != bin_count; ++ j)
        {
            float x = (float) j / (bin_count - 1);
            float y = values[j * channel_count + i] / (16 * 1024); // TODO parametrize

            glVertex2f(x, y);
        }
        glEnd();
    }
}

static Color channels_to_color(void)
{
    switch (channels)
    {
        default:
        case CHANNEL_ALL:   return WHITE;
        case CHANNEL_RED:   return RED;
        case CHANNEL_GREEN: return GREEN;
        case CHANNEL_BLUE:  return BLUE;
    }
}

static void display(void)
{
    glext_init();

    glClearColor(background.r, background.g, background.b, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    viewport_apply(viewport);
    viewport_enter_raster(viewport);

    glPushMatrix();
    glTranslatef(translation.x, translation.y, 0);
    glTranslatef(delta_translation.x, delta_translation.y, 0);
    glScalef(scale, scale, scale);

    static GLuint texture;
    if (texture == 0 || dirty_texture)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        Color contrast_color = color_scale(channels_to_color(), contrast);
        pixel_transfer_scale(contrast_color);
        pixel_map_correct_gamma(gamma_value);

        if (download_image->format.format == GL_LUMINANCE && false_colors)
        {
            Image * palette = palette_false(1024); // XXX
            ((Color *) palette->pixels)[0] = BLACK;

            Image * paletted_image = image_apply_palette(download_image, palette);
            texture_download(paletted_image);
            image_destroy(paletted_image);
            image_destroy(palette);
        }
        else
            texture_download_layer(download_image, layer);

        pixel_map_reset();
        pixel_transfer_reset();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);

        dirty_texture = 0;
    }
    else
        glBindTexture(GL_TEXTURE_2D, texture);

    glEnable(GL_TEXTURE_2D);

    Size size = download_image->format.size;

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glScalef(1.0 / size.x, 1.0 / size.y, 1);

    forward_matrix  = matrix_transformer();
    backward_matrix = matrix_invert(forward_matrix);

    color_apply(WHITE);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);           glVertex2f(0, 0);
    glTexCoord2f(size.x, 0);      glVertex2f(size.x, 0);
    glTexCoord2f(0, size.y);      glVertex2f(0, size.y);
    glTexCoord2f(size.x, size.y); glVertex2f(size.x, size.y);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_TEXTURE_2D);

    if (mode == ZOOMING)
    {
        color_apply(WHITE);
        draw_box(zoom_box);
    }

    if (scale >= 4)
    {
        color_apply(ORANGE);
        for (int i = 0; i != boxes.count; ++ i)
        {
            Box box = * (Box *) boxes.entries[i];
            
            glBegin(GL_LINE_LOOP);
            glVertex2f(box.min.x, box.min.y);
            glVertex2f(box.max.x, box.min.y);
            glVertex2f(box.max.x, box.max.y);
            glVertex2f(box.min.x, box.max.y);
            glEnd();
        }

        Box mouse_box;
        mouse_box.min = vector_floor(pick(mouse_position));
        mouse_box.max = vector_add(mouse_box.min, vector(1, 1, 0));

        color_apply(WHITE);
        draw_box(mouse_box);
    }
    else
    {
        color_apply(ORANGE);

        for (int i = 0; i != boxes.count; ++ i)
        {
            Box box = * (Box *) boxes.entries[i];
            Vector size = box_size(box);
            if (size.x == 1 && size.y == 1)
            {
                glPointSize(3.0);
                glBegin(GL_POINTS);
                vector_apply(box.min);
                glEnd();
            }
            else
            {
                glBegin(GL_LINE_LOOP);
                glVertex2f(box.min.x, box.min.y);
                glVertex2f(box.max.x, box.min.y);
                glVertex2f(box.max.x, box.max.y);
                glVertex2f(box.min.x, box.max.y);
                glEnd();
            }
        }
    }

    float MIN = 1E-2;
    float MAX = 1E+6;
    Box box =
    {
        {0, 0, 0},
        {size.x + MIN, size.y + MIN, size.z}
    };
    color_apply(WHITE);
    glBegin(GL_LINES);
    glVertex2f(box.min.x, -MAX); glVertex2f(box.min.x, +MAX);
    glVertex2f(box.max.x, -MAX); glVertex2f(box.max.x, +MAX);
    glVertex2f(-MAX, box.min.y); glVertex2f(+MAX, box.min.y);
    glVertex2f(-MAX, box.max.y); glVertex2f(+MAX, box.max.y);
    glEnd();

#if 0
    if (1)
    {
        color_apply(GREEN);
        Vector vertices[] = {{10, 10, 0}, {50, 30, 0}, {80, 60, 0}, {10, 200, 0}};
        draw_thick_lines(vertices, 4, 3);
    }
#endif

    glPopMatrix();

    viewport_leave_raster();

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 0
    viewport_enter_raster(viewport);
    draw_histogram(histogram); // TODO fix coordinate system
    viewport_leave_raster();
#endif

    if (scale >= 128)
        draw_pixel_content();

    Vector sample_position = vector_floor(picked_position); sample_position.z = layer;
    Size pixel_coordinates = {(int) floor(picked_position.x), (int) floor(picked_position.y), 0};

    font_begin(viewport);

    color_apply(WHITE);
    for (int i = 0; i != boxes.count; ++ i)
    {
        char buffer[256];
        sprintf(buffer, "%d ", i);
        Box box = * (Box *) boxes.entries[i];
        Vector position = {box.min.x, box.max.y, 0};
        font_render_local(buffer, position, ANCHOR_BOTTOM_RIGHT);
    }

    if (mouse_entered)
    {
        if (source_image->format.format == GL_LUMINANCE && source_image->format.type == GL_UNSIGNED_SHORT)
        {
            unsigned short color_index = image_sample_index(source_image, sample_position, BORDER_BLACK);
            draw_pixel_values_index(mouse_position, pixel_coordinates, color_index);
        }
        else
        {
            Color pixel_color = image_sample(download_image, sample_position, BORDER_BLACK);
            draw_pixel_values(mouse_position, pixel_coordinates, pixel_color);
        }
    }
    color_apply(WHITE);
    font_render_local(title, ORIGIN, ANCHOR_TOP_LEFT);
    font_render_local(image_size, vector(size.x, size.y, 0), ANCHOR_BOTTOM_LEFT);

    font_end();

    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    GLenum error = glGetError();
    if (error)
        printf(ANSI_RED "OpenGL error" ANSI_RESET ": %s\n", error_to_string(error));

    glutSwapBuffers();
}

static void timer(int value)
{
    if (! play)
        return;

    name_index = (name_index + 1) % names.count;
    update_image();
    dirty_texture = 1;

    glutTimerFunc(delay, timer, 42);
    glutPostRedisplay();
}

static void center(void)
{
    translation.x = (viewport.width  - scale * download_image->format.size.x) / 2,
    translation.y = (viewport.height - scale * download_image->format.size.y) / 2,
    translation.z = 0;

    dirty_texture = 1;
}

static void fit(int fill)
{
    float scale_x = (float) viewport.width  / download_image->format.size.x;
    float scale_y = (float) viewport.height / download_image->format.size.y;

    scale = fill ? fmax(scale_x, scale_y) : fmin(scale_x, scale_y);

    center();

    dirty_texture = 1;
}

#if 0
static Image * image_reformat_exr(Image const * image)
{
    Image_Format format = image->format;
    format.format = GL_RGBA;
    Image * image3 = image_new(format);
    image_add_alpha(image, image3);

    Image * target_image = image_retype(image3, GL_HALF_FLOAT_ARB);
    image_flip(target_image);

    image_destroy(image3);

    return target_image;
}
#endif

// XXX hack remove ASAP
/*static*/ void save_image(Image * im)
{
//    Image * image_2 = image_reformat_exr(im);
    Image * image_2 = im;
image_format_print(image_2->format);
    image_scale(image_2, 256);

    Image * image_3 = image_retype(image_2, GL_HALF_FLOAT_ARB);
    image_flip(image_3);
    exr_save(image_3, "diff.exr");
    printf("diff.exr\n");

    image_destroy(image_2);
}

static void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
        exit(EXIT_SUCCESS);

    if (key >= '0' && key <= '9')
    {
        int index = (key == '0') ? 9 : key - '1';
        if (index < names.count)
        {
            name_index = index;
            update_image();
            dirty_texture = 1;
            glutPostRedisplay();
        }
    }

    switch (key)
    {
        default:
            return;
        case 'P':
            //image_format_print(any_image->format);
            //printf("---\n");
            property_print(properties, property_count);
            break;

        case 'D':
            if (names.count >= 2)
            {
                char const * name1 = (char const *) names.entries[(name_index + 0) % names.count];
                char const * name2 = (char const *) names.entries[(name_index + 1) % names.count];

                Image * image2 = load_image(name2);
                if (! image2)
                {
                    warn("failed to load second image");
                    break;
                }

#if 1
                Image * diff_image = image_diff(download_image, image2);
#else
                printf("a\n");
                Image * diff_image = image_squared_error(download_image, image2, 0);
                printf("b\n");
                save_image(diff_image);
#endif
                if (diff_image)
                {
                    image_destroy(download_image);
                    download_image = diff_image;

                    sprintf(title, "Difference %s - %s", basename_(name1), basename_(name2));
                }
                else
                    warn("failed to diff images");

                image_destroy(image2);
            }
            break;

        case ' ': toggle(play); if (play) glutTimerFunc(delay, timer, 42); break;
        case 'C': center(); break;
        case 'w': fit(0); break;
        case 'W': fit(1); break;
        case 'g': toggle(false_colors); break;
        case 'G': gamma_value = gamma_value == 1.0 ? 2.2 : 1.0; break;
        case 'F': window_toggle_fullscreen(); break;
        case 'R': contrast = 1.0; scale = 1.0; translation = ORIGIN; channels = CHANNEL_ALL; break;
        case '=':
        case '+': contrast *= 2; break;
        case '-': contrast /= 2; break;
        case 'l': cycle     (layer, 0, download_image->format.size.z - 1); update_labels(); break;
        case 'L': cycle_down(layer, 0, download_image->format.size.z - 1); update_labels(); break;
        case 'c': cycle(channels, CHANNEL_ALL, CHANNEL_BLUE); break;
        case 'h': image_flip_horizontal(download_image); break;
        case 'v': image_flip(download_image); break;
        case 's': zoom(0.5); break;
        case 'S':
            {
                float old_scale = scale;
                zoom(2.0);
                // experiment
                if (viewport.width  == (int) floor(old_scale * download_image->format.size.x) &&
                    viewport.height == (int) floor(old_scale * download_image->format.size.y))
                    glutReshapeWindow(viewport.width * scale, viewport.height * scale);
            }
            break;
        case 'f': toggle(filter); break;
    }

    dirty_texture = 1;
    glutPostRedisplay();
}

static void special(int key, int x, int y)
{
    switch (key)
    {
        default: return;
        case GLUT_KEY_HOME:      name_index = 0; update_image(); break;
        case GLUT_KEY_END:       name_index = names.count - 1; update_image(); break;
        case GLUT_KEY_PAGE_DOWN: name_index += 10; if (name_index >= names.count) name_index = names.count - 1; update_image(); break;
        case GLUT_KEY_PAGE_UP:   name_index -= 10; if (name_index < 0) name_index = 0; update_image(); break;
        case GLUT_KEY_LEFT:      cycle_down(name_index, 0, names.count - 1); update_image(); break;
        case GLUT_KEY_RIGHT:     cycle     (name_index, 0, names.count - 1); update_image(); break; //schedule_value_action(0, 1, &alpha, 2.0); break;
        case GLUT_KEY_F11:       window_toggle_fullscreen(); break;
    }

    dirty_texture = 1;
    glutPostRedisplay();
}

static void initialize(int argc, char * argv[])
{
    background = ORANGE;

    char const * gamma_string = getenv("IV_GAMMA");
    if (gamma_string)
        gamma_value = atof(gamma_string);

    if (! variable_parse_arguments(variables, variable_count, argc, argv) || names.count == 0)
    {
        variable_usage(variables, variable_count, "iv");
        exit(EXIT_FAILURE);
    }

    half_initialize();
    font = font_open("Arial", 14);

    update_image();

    if (play)
        glutTimerFunc(delay, timer, 42);
}

static void reshape(int width, int height)
{
    viewport.width  = width;
    viewport.height = height;

    if (width  >= scale * download_image->format.size.x ||
        height >= scale * download_image->format.size.y)
        center();
}

static void mouse(int button, int state, int x, int y)
{
    Vector position = {x, viewport.height - 1 - y, 0};
    mouse_position = position;

    switch (button)
    {
        default: return;

        case GLUT_LEFT_BUTTON:

            if (mode == NORMAL)
                grabbed_position = position;

            mode = (state == GLUT_DOWN) ? GRABBED : NORMAL;
            glutSetCursor(mode == GRABBED ? GLUT_CURSOR_INFO : GLUT_CURSOR_INHERIT);

            if (mode == NORMAL)
            {
                translation = vector_add(translation, delta_translation);
                delta_translation = ORIGIN;
            }

            break;

        case GLUT_RIGHT_BUTTON:

            if (mode == NORMAL)
                zoom_box.min = zoom_box.max = vector_floor(pick(position));

            mode = (state == GLUT_DOWN) ? ZOOMING : NORMAL;
            glutSetCursor(mode == ZOOMING ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);

            if (mode == NORMAL)
            {
                // TODO fix zoom rectangle
                zoom_box = box_fix(zoom_box);
                picked_position = box_center(zoom_box);

                Vector center = {viewport.width / 2.0, viewport.height / 2.0, 0};
                Vector center2 = pick(center);

                Vector t2 = vector_sub(picked_position, center2);
                t2 = t2; // XXX

                Vector size = box_size(zoom_box);
                float factor = fmin(viewport.width / size.x, viewport.height / size.y);

                scale = factor;
            }

            break;

        case GLUT_SCROLL_UP:   zoom(1.0 * zoom_factor); break; 
        case GLUT_SCROLL_DOWN: zoom(1.0 / zoom_factor); break;
    }

    glutPostRedisplay();
}

static void motion(int x, int y)
{
    Vector position = {x, viewport.height - 1 - y, 0};
    mouse_position  = position;
    picked_position = pick(position);

    switch (mode)
    {
        case NORMAL:  break;
        case GRABBED: delta_translation = vector_sub(position, grabbed_position); break;
        case ZOOMING: zoom_box.max = vector_floor(pick(mouse_position)); break;
    }

    glutPostRedisplay();
}

static void passive_motion(int x, int y)
{
    Vector position = {x, viewport.height - 1 - y, 0};
    mouse_position  = position;
    picked_position = pick(position);

    glutPostRedisplay();
}

static void entry(int state)
{
    switch (state)
    {
        case GLUT_LEFT:    mouse_entered = 0; break;
        case GLUT_ENTERED: mouse_entered = 1; break;
    }

    glutPostRedisplay();
}

static void idle(void)
{
     if (! action_count())
     {
         time_sleep(20);
         return;
     }

     action_process();
     glutPostRedisplay();
}

int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
    initialize(argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(download_image->format.size.x, download_image->format.size.y);

    glutCreateWindow(title);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutEntryFunc(entry);
    glutPassiveMotionFunc(passive_motion);

    if (fullscreen)
        glutFullScreen();

    glutMainLoop();

    return EXIT_FAILURE;
}

