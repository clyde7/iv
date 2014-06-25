#include <stdlib.h>

#ifdef PNG
#include <png.h>
#include <zlib.h>

#include "error.h"
#include "file.h"
#include "file_image.h"
#include "image.h"
#include "memory.h"
#include "opengl.h"
#include "string.h"

typedef enum PNGerror
{
    NOT_A_PNG_FILE = 1,
    FAILED_TO_CREATE_PNG_INFO,
    FAILED_TO_CREATE_PNG_READER,
    FAILED_TO_CREATE_PNG_WRITER,
    FAILED_TO_WRITE
}
PNGerror;

char const * PNG_MIME = "image/png";
static char * error_message;
static jmp_buf jump_buffer;

static GLenum translate_format(int color_type)
{
    switch (color_type)
    {
        case PNG_COLOR_TYPE_GRAY:
            return GL_LUMINANCE;

        case PNG_COLOR_TYPE_GRAY_ALPHA:
            return GL_LUMINANCE_ALPHA;

        case PNG_COLOR_TYPE_PALETTE:
            return GL_COLOR_INDEX;

        case PNG_COLOR_TYPE_RGB:
            return GL_RGB;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            return GL_RGBA;
    }

    fprintf(stderr, "color type %d not supported\n", color_type);
    return 0;
}

static GLenum translate_type(int bit_depth)
{
    switch (bit_depth)
    {
        case 8:  return GL_UNSIGNED_BYTE;
        case 16: return GL_UNSIGNED_SHORT;
    }

    fprintf(stderr, "bit depth %d not supported\n", bit_depth);
    return 0;
}

static void handle_warning(png_structp reader, png_const_charp message)
{
    fprintf(stderr, "warning: %s\n", message);
}

static void handle_error(png_structp reader, png_const_charp message)
{
    error_message = strdup(message);
    longjmp(jump_buffer, 1);
}

Image_Format png_format(FILE * file)
{
    png_structp reader = NULL;
    png_infop info = NULL;
    png_bytep * rows = NULL;
    int value;

    /* handle error */
    if ((value = setjmp(jump_buffer)) != 0)
    {
        printf("Error %d\n", value);
        
        free(rows);

        if (reader)
            png_destroy_read_struct(&reader, &info, NULL);

        if (file)
            fclose(file);

        Image_Format format = {0, 0, {0, 0, 0}};
        return format;
    }

    png_byte header[8];

    fread(header, 1, sizeof(header), file);
    if (png_sig_cmp(header, 0, sizeof(header)) != 0)
        longjmp(jump_buffer, NOT_A_PNG_FILE);

    reader = png_create_read_struct(PNG_LIBPNG_VER_STRING,
//      (png_voidp) error_pointer, error_handler, warning_handler);
        (png_voidp) NULL, handle_error, handle_warning);
    if (reader == NULL)
        longjmp(jump_buffer, FAILED_TO_CREATE_PNG_READER);

    info = png_create_info_struct(reader);
    if (info == NULL)
        longjmp(jump_buffer, FAILED_TO_CREATE_PNG_INFO);

    png_init_io(reader, file);
    png_set_sig_bytes(reader, sizeof(header));

    png_read_info(reader, info);

    unsigned width = (unsigned) png_get_image_width(reader, info);
    unsigned height = (unsigned) png_get_image_height(reader, info);
    int color_type = png_get_color_type(reader, info);
    int bit_depth  = png_get_bit_depth(reader, info);

    GLenum type   = translate_type(bit_depth);
    GLenum format = translate_format(color_type);

    /* removed reading */

    png_destroy_read_struct(&reader, &info, NULL);
    fclose(file);

    Image_Format image_format = {type, format, {width, height, 1}};
    return image_format;
}

Image * png_load_flip(FILE * file, int flip)
{
    png_structp reader = NULL;
    png_infop info = NULL;
    png_bytep * rows = NULL;
    int value;

    /* handle error */
    if ((value = setjmp(jump_buffer)) != 0)
    {
        printf("Error %d\n", value);
        
        free(rows);

        if (reader)
            png_destroy_read_struct(&reader, &info, NULL);

        if (file)
            fclose(file);

        return NULL;
    }

    png_byte header[8];

    fread(header, 1, sizeof(header), file);
    if (png_sig_cmp(header, 0, sizeof(header)) != 0)
        longjmp(jump_buffer, NOT_A_PNG_FILE);

    reader = png_create_read_struct(PNG_LIBPNG_VER_STRING,
//      (png_voidp) error_pointer, error_handler, warning_handler);
        (png_voidp) NULL, handle_error, handle_warning);
    if (reader == NULL)
        longjmp(jump_buffer, FAILED_TO_CREATE_PNG_READER);

    info = png_create_info_struct(reader);
    if (info == NULL)
        longjmp(jump_buffer, FAILED_TO_CREATE_PNG_INFO);

    png_init_io(reader, file);
    png_set_sig_bytes(reader, sizeof(header));

    png_read_info(reader, info);

    // read properties

    png_text * texts;
    int property_count;
    png_get_text(reader, info, &texts, &property_count);

    Property * properties = malloc_array(Property, property_count);
    for (int i = 0; i != property_count; ++ i)
    {
        properties[i].key = strdup(texts[i].key);
        properties[i].value = strdup(texts[i].text);
    }

#if 0
    if (1)
    {    
        double gamma;
        png_get_gAMA(reader, info, &gamma);
        printf("gamma = %f\n", gamma);
    }
#endif

    // TODO return properties somewhere

    unsigned  const width  = (unsigned) png_get_image_width(reader, info);
    unsigned  const height = (unsigned) png_get_image_height(reader, info);
    int const color_type = png_get_color_type(reader, info);
    int const bit_depth  = png_get_bit_depth(reader, info);

    GLenum type   = translate_type(bit_depth);
    GLenum format = translate_format(color_type);
    unsigned components = format_to_size(format);
    Image_Format image_format = {type, format, {width, height, 1}};

    GLsizei byte_count = image_format_bytes(image_format);
    GLubyte * pixels = (GLubyte *) malloc(byte_count);

    rows = (png_bytep *) malloc(components * width * (bit_depth / 8) * sizeof(png_bytep)); /* XXX may need bit_depth / 8 */
    unsigned i;
    for (i = 0; i < height; ++ i)
    {
        int row = flip ? (height - 1 - i) : i;
        rows[i] = &pixels[row * components * width * (bit_depth / 8)];
    }

    png_read_image(reader, rows);
    png_read_end(reader, NULL);

    free(rows);
    png_destroy_read_struct(&reader, &info, NULL);
    fclose(file);

    return image_create(image_format, pixels);
}

Image * png_load(FILE * file)
{
    return png_load_flip(file, 1);
}

/* writer */

static int translate_bit_depth(GLenum format, GLenum type)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE:  return 8;
        case GL_UNSIGNED_SHORT: return 16;
    }

    return 8;
}

static int translate_color_type(GLenum format, GLenum type)
{
    switch (format)
    {
        case GL_LUMINANCE:
            return PNG_COLOR_TYPE_GRAY;

        case GL_LUMINANCE_ALPHA:
            return PNG_COLOR_TYPE_GRAY_ALPHA;

        case GL_COLOR_INDEX:
            return PNG_COLOR_TYPE_PALETTE;

        case GL_RGB:
            return PNG_COLOR_TYPE_RGB;

        case GL_RGBA:
            return PNG_COLOR_TYPE_RGB_ALPHA;
    }

    fprintf(stderr, "format %lx not supported\n", (unsigned long) format);
    return 0;
}

static void handle_progress(png_structp writer, png_uint_32 row, int pass)
{
//    if (row == writer->height -1)
//        fprintf(stderr, "saved\n");
}

int png_save_with_properties(Image const * image, FILE * file, Property const properties[], int property_count)
{
    GLvoid const * image_pixels = image->pixels;
    Image_Format format = image->format;
    GLenum image_type = format.type;
    GLenum image_format = format.format;
    GLsizei image_width = format.size.x;
    GLsizei image_height = format.size.y;

    png_structp writer = NULL;
    png_infop info = NULL;
    png_bytepp rows = NULL;
    int value = 0;
    int i;

    /* handle error */
    if ((value = setjmp(jump_buffer)) != 0)
    {
        printf("Error %d\n", value);

        free(rows);

        if (error_message)
        {
            printf("%s\n", error_message);
            free(error_message);
            error_message = NULL;
        }

        if (writer)
            png_destroy_write_struct(&writer, &info);

        if (file)
            fclose(file);

        return EXIT_FAILURE;
    }
    
    writer = png_create_write_struct(PNG_LIBPNG_VER_STRING,
        (png_voidp) NULL, handle_error, handle_warning);
    if (writer == NULL)
        longjmp(jump_buffer, FAILED_TO_CREATE_PNG_WRITER);

    info = png_create_info_struct(writer);
    if (info == NULL)
        longjmp(jump_buffer, FAILED_TO_CREATE_PNG_INFO);

    png_init_io(writer, file);
    png_set_write_status_fn(writer, handle_progress);

    png_set_filter(writer, 0, PNG_FILTER_VALUE_NONE);
    png_set_compression_level(writer, Z_BEST_COMPRESSION);
    png_set_compression_mem_level(writer, 8);
    png_set_compression_strategy(writer, Z_DEFAULT_STRATEGY);
    png_set_compression_window_bits(writer, 15);
    png_set_compression_method(writer, 8);
    png_set_compression_buffer_size(writer, 8192);

    // add properties

    png_text texts[256];
    error_check(property_count > 256, "too many properties");

    for (i = 0; i != property_count; ++ i)
    {
        texts[i].compression = PNG_TEXT_COMPRESSION_NONE;
        texts[i].key  = (char *) properties[i].key;
        texts[i].text = (char *) properties[i].value;
    }

    png_set_text(writer, info, texts, property_count);

    // gamma

#if 0
    if (1)
    {
        double gamma = 1.7;
        png_set_gAMA(writer, info, gamma);
    }
    double screen_gamma = 1.7;
    double file_gamma = 1.0;
    png_set_gamma(writer, screen_gamma, file_gamma);
#endif
    //png_set_gamma(writer, 1.0, 1.0 / 1.7);
    //png_set_gamma(writer, 1.7, 1.0);
    //png_set_gamma(writer, 1.7, 1.0 / 1.7);
    //png_set_sRGB(writer, info, PNG_sRGB_INTENT_RELATIVE);

    //

    int bit_depth = translate_bit_depth(image_format, image_type);
    int color_type = translate_color_type(image_format, image_type);
    unsigned components = format_to_size(image_format);
    png_set_IHDR(writer, info, image_width, image_height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    rows = (png_bytepp) malloc(image_height * sizeof(png_bytep));
    for (i = 0; i < image_height; ++ i)
    {
        rows[i] = &((GLubyte *) image_pixels)[(image_height - 1 - i) * image_width * components * bit_depth / 8];
    }

    png_set_rows(writer, info, rows);
    png_write_png(writer, info, PNG_TRANSFORM_IDENTITY, NULL);

    free(rows);
    png_destroy_write_struct(&writer, &info);
    fclose(file);

    return EXIT_SUCCESS;
}

int png_save(Image const * image, FILE * file)
{
    return png_save_with_properties(image, file, NULL, 0);
}


void png_snapshot_name(char const name[], Viewport viewport, GLenum format, Property const properties[], int property_count)
{
    FILE * file = fopen(name, "wb");

    Image * image = image_read(viewport, format, GL_UNSIGNED_BYTE);
    if (format == GL_DEPTH_COMPONENT)
        image->format.format = GL_LUMINANCE;

    png_save_with_properties(image, file, properties, property_count);

    printf("saved image \"%s\"\n", name);
//    property_print(properties, property_count);

    image_destroy(image);
}

void png_snapshot(char const basename[], Viewport viewport, GLenum format, Property const properties[], int property_count)
{
    char pattern[256], name[256];
    sprintf(pattern, "%s-%%04d.png", basename);
    file_unique_name(name, pattern);

    png_snapshot_name(name, viewport, format, properties, property_count);
}

#endif
