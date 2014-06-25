#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <gif_lib.h>

#include "error.h"
#include "file_image.h"

#ifdef CYGWIN
int fileno(FILE *);
#endif

char const * GIF_MIME = "image/gif";

static int has_transparency(SavedImage const * image, unsigned * index)
{
    int i;

    assert(image);

    int const count = image->ExtensionBlockCount;
    for (i = 0; i < count; ++ i)
    {
        ExtensionBlock * extension = &(image->ExtensionBlocks[i]);

        if ((extension->Function == GRAPHICS_EXT_FUNC_CODE) &&
            (extension->Bytes[0] & 0x01))
        {
            *index = extension->Bytes[3];
            return 1;
        }
    }

    return 0;
}

#if 0
static int get_delay_time(SavedImage const * image)
{
    int i;

    int const count = image->ExtensionBlockCount;
    for (i = 0; i < count; ++ i)
    {
        ExtensionBlock * extension = &(image->ExtensionBlocks[i]);

        if (extension->Function == GRAPHICS_EXT_FUNC_CODE)
        {
            return
                extension->Bytes[1] +
                (extension->Bytes[2] << 8);
        }
    }

    return 0;
}
#endif

static int mapInterlacedRow(int row, int height)
{
    // offsets = 0, 4, 2, 1
    // strides = 8, 8, 4, 2

    if ((row + 8) % 8 == 0)
    {
        return (row - 0) / 8;
    }
    else if (((row + 4) % 8) == 0)
    {
        return (height + 7) / 8 + (row - 4) / 8 ;
    }
    else if (((row + 2) % 4) == 0)
    {
        return (height + 7) / 8 + (height + 7 - 4) / 8 + (row - 2) / 4;
    }
    else if (((row + 1) % 2) == 0)
    {
        return (height + 7) / 8 + (height + 7 - 4) / 8 + (height + 3 - 2) / 4
            + (row - 1) / 2;
    }

    exit(EXIT_FAILURE);
}

static void pasteImage(GifFileType const * file, SavedImage const * image,
    GLubyte pixels[], GLenum format, int const size[3], int frame_index)
{
    GifImageDesc const desc = image->ImageDesc;
    GLint const framePosition[2] = {desc.Left, desc.Top};
    GLsizei const frameSize[2] = {desc.Width, desc.Height};
    int const interlaced = desc.Interlace;

    unsigned int transparency_index = -1;
    int const transparency = has_transparency(image, &transparency_index);

    int const componentSize = format_to_size(format);
    int const rowSize = format_to_size(format) * size[0];

    for (int i = 0; i != frameSize[1]; i++)
    {
        const int dstIndex =
            (framePosition[0] + 0) * componentSize +
            (framePosition[1] + i) * rowSize;

        GLubyte * rowPixels = &pixels[dstIndex];

        for (int j = 0; j != frameSize[0]; j++) {
            const int rowIndex = interlaced
                ? mapInterlacedRow(i, frameSize[1])
                : i;

            unsigned char const index = (int) (image->RasterBits[frameSize[0] * rowIndex + j]);

            GifColorType const * color = &file->SColorMap->Colors[index];

            if (format == GL_RGBA)
            {
                if (frame_index != 0 && transparency && (index == transparency_index))
                    continue;

                rowPixels[j * 4 + 0] = color->Red;
                rowPixels[j * 4 + 1] = color->Green;
                rowPixels[j * 4 + 2] = color->Blue;
                rowPixels[j * 4 + 3] = transparency && (index == transparency_index) ? 0 : 255;
            }
            else
            {
                rowPixels[j * 3 + 0] = color->Red;
                rowPixels[j * 3 + 1] = color->Green;
                rowPixels[j * 3 + 2] = color->Blue;
            }
        }
    }
}

Image * gif_load(FILE * file_)
{
    int i;
    int file_descriptor = fileno(file_);
    GifFileType * file = DGifOpenFileHandle(file_descriptor);

    if (file == NULL)
    {
        fprintf(stderr, "failed to open GIF file\n");
        return NULL;
    }

    if (DGifSlurp(file) == GIF_ERROR)
    {
        fprintf(stderr, "failed to read GIF file\n");
        if (file)
            DGifCloseFile(file);
        return NULL;
    }
        
    int size[3] = {file->Image.Width, file->Image.Height, file->ImageCount};

//      don't care
//      const bool interlaced = file->Image.Interlace;

    // determine transparency - XXX should only check first image but all
    // pixels
    GLenum format = GL_RGB;
    for (i = 0; i != size[2]; ++ i)
    {
        unsigned int index;
        if (has_transparency(&file->SavedImages[i], &index))
        {
            format = GL_RGBA;
            break;
        }
    }

    /* determine real size - workaround */
    for (i = 0; i != size[2]; ++ i)
    {
        SavedImage const * image = &file->SavedImages[i];
        GifImageDesc desc = image->ImageDesc;

        if (desc.Left + desc.Width > size[0])
            size[0] = desc.Left + desc.Width;

        if (desc.Top + desc.Height > size[1])
            size[1] = desc.Top + desc.Height;
    }

    Image_Format image_format = {GL_UNSIGNED_BYTE, format, {size[0], size[1], size[2]}};
    GLsizei bytes = image_format_bytes(image_format);
    const int frame_size = size[0] * size[1] * format_to_size(format);

    GLubyte * pixels = (GLubyte *) malloc(bytes);

    // copy pixels
    for (i = 0; i != size[2]; ++ i)
    {
        SavedImage const * image = &file->SavedImages[i];

        const int frame_index = i * frame_size;
        GLubyte * frame_pixels = &pixels[frame_index];

        // copy over last image if not fully covered
//          if (l != 0 && (frameSize[0] != size[0] || frameSize[1] != size[1]))
        if (i != 0)
            memcpy(frame_pixels, &frame_pixels[-frame_size], frame_size);
        pasteImage(file, image, frame_pixels, format, size, i);
    }

    DGifCloseFile(file);

    return image_create(image_format, pixels);
}

void gif_save(Image const * image, FILE * file_, int loop_count, float delay)
{
    Image_Format format = image->format;

    error_check(format.type != GL_UNSIGNED_BYTE, "type must be unsigned byte");
    error_check(format.format != GL_LUMINANCE, "type must be luminance");

    Size size = format.size;
    unsigned char const * pixels = (unsigned char *) image->pixels;

    int file_descriptor = fileno(file_);
    GifFileType * file = EGifOpenFileHandle(file_descriptor);
    EGifSetGifVersion("89a");

    ColorMapObject * color_map = MakeMapObject(256, NULL);
    for (int i = 0; i != 256; ++ i)
    {
        color_map->Colors[i].Red   = i;
        color_map->Colors[i].Green = i;
        color_map->Colors[i].Blue  = i;
    }
    EGifPutScreenDesc(file, size.x, size.y, 256, -1, color_map);
    FreeMapObject(color_map);

    char netscape_ext[12] = "NETSCAPE2.0";
    EGifPutExtensionFirst(file, APPLICATION_EXT_FUNC_CODE, 11, netscape_ext);

    char subblock[3];
    subblock[0] = 1;
    subblock[1] = loop_count % 256;
    subblock[2] = loop_count / 256;
    EGifPutExtensionLast(file,  APPLICATION_EXT_FUNC_CODE, 3, subblock);

    unsigned char graphics_control[4];
    graphics_control[0] = 0; // disposal
    graphics_control[1] = (int) round(delay * 100) % 256;
    graphics_control[2] = (int) round(delay * 100) / 256;
    graphics_control[3] = 0xff; // no transparency
    EGifPutExtension(file, GRAPHICS_EXT_FUNC_CODE, 4, graphics_control);

    for (int i = 0; i != size.z; ++ i)
    {
        EGifPutImageDesc(file, 0, 0, size.x, size.y, 0, NULL);
        for (int j = 0; j != size.y; ++ j)
        {
            EGifPutLine(file, (unsigned char *) &pixels[i * size.y * size.x + j * size.x], size.x);
        }
    }

//    EGifPutComment(file, "progressive image denoising");

    EGifCloseFile(file);
}

