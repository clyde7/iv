#ifndef IMAGE_H
#define IMAGE_H

#include "opengl.h"
#include "property.h"
#include "size.h"
#include "viewport.h"

extern char const * BOB_MIME, * GIF_MIME, * JPEG_MIME, * MPO_MIME, * PNG_MIME, * PPM_MIME, * PGM_MIME, * PNM_MIME, * RAW_MIME, * TIFF_MIME;
#ifdef EXR
extern char const * EXR_MIME;
#endif

struct Image;

typedef struct {GLenum type, format; Size size;} Image_Format;
typedef struct Image {Image_Format format; void * pixels;
struct Image * palette; void * device;} Image;

GLenum  size_to_format(GLsizei);
GLsizei format_to_size(GLenum);
int image_type_to_size(GLenum);

int  image_format_equal(Image_Format, Image_Format);
void image_format_print(Image_Format);
int  image_format_bytes(Image_Format);
int  image_format_pixels(Image_Format);
int  image_format_pixel_size(Image_Format);
Size image_format_total_size(Image_Format);
Size image_format_stride(Image_Format);
int  image_format_dimension(Image_Format);

Image * image_new(Image_Format);
Image * image_copy(Image const *);
Image * image_create(Image_Format, void * pixels);
Image * image_open(char const name[]);
Image * image_read(Viewport, GLenum format, GLenum type);
Image * image_stack(Image const * stack[], int count);
Image * image_grey_gradient(GLenum type);

void image_destroy(Image *);
void image_clear(Image *);
void image_store_unpack(Image const *);
void image_store_unpack_region(Image, GLint const position[3], Size);
void image_store_pack(Image, Size);
void image_store_pack_region(Image, GLint const position[3], Size);

void image_print(Image const *);
void image_draw(Image const *);
void image_draw_layer(Image const *, int);

void image_flip(Image *);

Property * image_properties(char const name[], unsigned * count);

char const * file_guess_mime_type(char const file_name[]);

#endif
