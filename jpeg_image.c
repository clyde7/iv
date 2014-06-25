#ifdef CYGWIN
#define XMD_H
#define HAVE_BOOLEAN
typedef int boolean_;
#define boolean boolean_
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#ifdef JPEG
#include <jpeglib.h>

#ifdef CYGWIN
#undef boolean
#endif

#include "file.h"
#include "file_image.h"

char const * JPEG_MIME = "image/jpeg";

static char const * message;
static jmp_buf jump_buffer;

GLenum components_to_format(int components)
{
    switch (components)
    {
        case 1: return GL_COLOR_INDEX;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
    }

    fprintf(stderr, "unsupported number of components %d\n", components);
    exit(EXIT_FAILURE);
}

METHODDEF(void) errorHandler(j_common_ptr decompressor)
{
    struct jpeg_error_mgr const * const errorManager = decompressor->err;
    message = errorManager->jpeg_message_table[errorManager->msg_code];
    longjmp(jump_buffer, 1);
}

METHODDEF(void) outputMessage(j_common_ptr decompressor)
{
    struct jpeg_error_mgr const * const errorManager = decompressor->err;
    message = errorManager->jpeg_message_table[errorManager->msg_code];
    longjmp(jump_buffer, 1);
}

Image * jpeg_load(FILE * file)
{
    int i;

    if (setjmp(jump_buffer) != 0)
    {
        fprintf(stderr, "error: %s\n", message);
        fclose(file);
        return NULL;
    }

    struct jpeg_decompress_struct decompressor;
    struct jpeg_error_mgr errorManager;

    decompressor.err = jpeg_std_error(&errorManager);
    errorManager.error_exit = errorHandler;
/*  errorManager.emit_message = emitMessage; */
    errorManager.output_message = outputMessage;

    jpeg_create_decompress(&decompressor);
    jpeg_stdio_src(&decompressor, file);
    jpeg_read_header(&decompressor, TRUE);
    jpeg_start_decompress(&decompressor);

    int components = decompressor.output_components;
    int width = decompressor.output_width;
    int height = decompressor.output_height;
    Image_Format image_format = {GL_UNSIGNED_BYTE, components_to_format(components), {width, height, 1}};

    /* allocate buffer (freed by jpeg_finish_decompress) */
    JSAMPARRAY array = (*decompressor.mem->alloc_sarray)
        ((j_common_ptr) &decompressor, JPOOL_IMAGE,
        width * components, height);

    for (i = 0; i != height;
        i += jpeg_read_scanlines(&decompressor, &array[i], height - i));

    GLubyte * pixels = (GLubyte *) malloc(components * width * height);
    for (i = 0; i != height; ++ i)
    {
        memcpy(&pixels[(height - 1 - i) * width * components], array[i], width * components);
    }

    jpeg_finish_decompress(&decompressor);
    jpeg_destroy_decompress(&decompressor);
    fclose(file);

    return image_create(image_format, pixels);
}

static int format_to_color_space(GLenum format)
{
    switch (format)
    {
        case GL_RGB:
            return JCS_RGB;

        case GL_DEPTH_COMPONENT:
        case GL_LUMINANCE:
        case GL_ALPHA:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
            return JCS_GRAYSCALE;
    }

    fprintf(stderr, "unsupported JPEG format \"%d\"\n", format);
    exit(EXIT_FAILURE);
}

void jpeg_save(Image const * image, FILE * file)
{
    struct jpeg_error_mgr errorManager;
    errorManager.error_exit = errorHandler;
/*  errorManager.emit_message = emitMessage; */
    errorManager.output_message = outputMessage;

    struct jpeg_compress_struct compressor;
    compressor.err = jpeg_std_error(&errorManager);

    jpeg_create_compress(&compressor);
    jpeg_stdio_dest(&compressor, file);

    Image_Format format = image->format;
    GLubyte * pixels = (GLubyte *) image->pixels;

    compressor.image_width = format.size.x;
    compressor.image_height = format.size.y;
    compressor.input_components = format_to_size(format.format);
    compressor.in_color_space = (J_COLOR_SPACE) format_to_color_space(format.format);

    jpeg_set_defaults(&compressor);

    jpeg_start_compress(&compressor, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride = format.size.x * 3;

    while (compressor.next_scanline < (unsigned) format.size.y)
    {
         row_pointer[0] = &pixels[(format.size.y - 1 - compressor.next_scanline) * row_stride];
         jpeg_write_scanlines(&compressor, row_pointer, 1);
    }

    jpeg_finish_compress(&compressor);
    jpeg_destroy_compress(&compressor);

    fclose(file);
}

void jpeg_snapshot(char const basename[], Viewport viewport, GLenum format)
{
    Image * image = image_read(viewport, format, GL_UNSIGNED_BYTE);

    if (format == GL_DEPTH_COMPONENT)
        image->format.format = GL_LUMINANCE;

    char pattern[256], name[256];
    sprintf(pattern, "%s-%%04d.jpg", basename);
    file_unique_name(name, pattern);

    FILE * file = fopen(name, "wb");
    jpeg_save(image, file);

    printf("saved \"%s\"\n", name);

    image_destroy(image);
}
#endif
