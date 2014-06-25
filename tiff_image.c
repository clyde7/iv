#include <stdlib.h>

#ifdef TIFF
#include <tiffio.h>

#include "file_image.h"

#ifdef CYGWIN
int fileno(FILE *);
#endif

char const * TIFF_MIME = "image/tiff";

static void handle_warning(char const module[], char const format[], va_list arguments)
{
    char string[256];
    vsprintf(string, format, arguments);
    fprintf(stderr, "warning: %s\n", string);
}

static void handle_error(char const module[], char const format[], va_list arguments)
{
    char string[256];
    vsprintf(string, format, arguments);
    fprintf(stderr, "error: %s\n", string);
    exit(EXIT_FAILURE);
}

Image * tiff_load(FILE * file)
{
    int i;

    TIFFSetErrorHandler(handle_error);
    TIFFSetWarningHandler(handle_warning);

    TIFF * tiff = TIFFFdOpen(fileno(file), "dummy", "rb");
    if (tiff == NULL)
    {
        fprintf(stderr, "failed to open TIFF file\n");
        return NULL;
    }

    GLsizei size[3];
    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &size[0]);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &size[1]);

    GLsizei dir_count = 0;
    do {
        ++ dir_count;
    }
    while (TIFFReadDirectory(tiff));
    size[2] = dir_count;

    Image_Format format = {GL_UNSIGNED_BYTE, GL_RGBA, {size[0], size[1], size[2]}};

    Size total_size = image_format_total_size(format);
    GLubyte * pixels = (GLubyte *) malloc(total_size.z);

    for (i = 0; i < size[2]; i++)
    {
        TIFFSetDirectory(tiff, i);
        int index = i * total_size.y;
        if (! TIFFReadRGBAImage(tiff, size[0], size[1], (uint32 *) &pixels[index], 0))
        {
            fprintf(stderr, "failed to read TIFF file\n");
            TIFFClose(tiff);
            return NULL;
        }
    }
        
    TIFFClose(tiff);

    return image_create(format, pixels);
}
#endif
