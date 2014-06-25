#include <cstdio>
#include <set>

#ifdef EXR
#include <ImathBox.h>
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfFloatAttribute.h>

#include <ImfChannelList.h>
#endif

extern "C"
{
    #include "error.h"
    #include "file_image.h"
    #include "memory.h"
}

char const * EXR_MIME = "image/x-exr";

#ifdef EXR

using namespace std;
using namespace Imf;
using namespace Imath;

static RgbaChannels type_to_channels(GLenum format)
{
    switch (format)
    {
        case GL_RGBA:            return WRITE_RGBA;
        case GL_RGB:             return WRITE_RGB;
        case GL_LUMINANCE:       return WRITE_Y;
        case GL_LUMINANCE_ALPHA: return WRITE_YA;
    }

    return WRITE_RGBA;
}

static int channel_count(Header const & header)
{
    ChannelList const & channels = header.channels();
    set<string> layer_names;
    channels.layers(layer_names);

    return layer_names.size() ? layer_names.size() : 1;
}

int exr_layer_count(char const file_name[])
{
    RgbaInputFile file(file_name);
    return channel_count(file.header());
}

char const * exr_layer_name(char const file_name[], int index)
{
    RgbaInputFile file(file_name);

    ChannelList const & channels = file.header().channels();
    set<string> layer_names;
    channels.layers(layer_names);

    set<string>::const_iterator i = layer_names.begin();
    for (int j = 0; j != index; ++ i, ++ j);

    return strdup(i->c_str()); // leak
}

static void dump_channels(RgbaInputFile & file)
{
    ChannelList const & channels = file.header().channels();
    set<string> layer_names;
    channels.layers(layer_names);

    for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++ i)
    {
        cout << i.name() << endl;
    }

    cout << "channels = " << endl;

    for (set<string>::const_iterator i = layer_names.begin(); i != layer_names.end(); ++ i)
    {
        cout << "layer " << *i << endl;

        ChannelList::ConstIterator layer_begin, layer_end;
        channels.channelsInLayer(*i, layer_begin, layer_end);

        for (ChannelList::ConstIterator j = layer_begin; j != layer_end; ++ j)
        {
            cout << "channel " << j.name() << endl;
        }
    }
}

#if 0
static Image * exr_load_single(char const name[])
{
    RgbaInputFile file(name);

    if (0)
        dump_channels(file);

    Box2i box = file.dataWindow();
    int width  = box.max.x - box.min.x + 1;
    int height = box.max.y - box.min.y + 1;

    Image_Format format = {GL_HALF_FLOAT_ARB, GL_RGBA, {width, height, 1}};
    Image * image = image_new(format);

    file.setFrameBuffer((Rgba *) image->pixels, 1, width);
    file.readPixels(box.min.y, box.max.y);

    image_flip(image);

    return image;
}
#else
static Image * exr_load_single(char const name[])
{
    InputFile file(name);
    Header const & header = file.header();

    Box2i box = header.dataWindow();
    int width  = box.max.x - box.min.x + 1;
    int height = box.max.y - box.min.y + 1;

    ChannelList const & channels = header.channels();
    int channel_count = 0;
    for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++ i)
    {
//         printf("channel name = \"%s\"\n", i.name());
         ++ channel_count;
    }
//    printf("channel count = %d\n", channel_count);

    Image_Format format = {GL_HALF_FLOAT_ARB, GL_LUMINANCE, {width, height, 1}};
    format.format = size_to_format(channel_count);
    Image * image = image_new(format);

    unsigned short * pixels = (unsigned short *) image->pixels;
    Size stride = image_format_stride(image->format);
//printf("stride.x = %d\n", stride.x);
    FrameBuffer buffer;

    switch (channel_count)
    {
        case 1: buffer.insert("Y", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
                break;

        case 2: buffer.insert("Y", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
                buffer.insert("A", Slice(HALF, (char *) &pixels[1], stride.x, stride.y));
                break;

        case 3: buffer.insert("R", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
                buffer.insert("G", Slice(HALF, (char *) &pixels[1], stride.x, stride.y));
                buffer.insert("B", Slice(HALF, (char *) &pixels[2], stride.x, stride.y));
                break;

        case 4: buffer.insert("R", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
                buffer.insert("G", Slice(HALF, (char *) &pixels[1], stride.x, stride.y));
                buffer.insert("B", Slice(HALF, (char *) &pixels[2], stride.x, stride.y));
                buffer.insert("A", Slice(HALF, (char *) &pixels[3], stride.x, stride.y));
                break;
    }
    file.setFrameBuffer(buffer);
    file.readPixels(box.min.y, box.max.y);

    image_flip(image);

    return image;
}
#endif
static void header_add_properties(Header & header, Property const properties[], int property_count)
{
    for (int i = 0; i != property_count; ++ i)
    {
        Property const * property = &properties[i];
        header.insert(property->key, StringAttribute((char const *) property->value));
    }
}

void exr_save_with_properties_single(Image const * image, char const name[], Property const properties[], int property_count)
{
    Image_Format format = image->format;

    error_check(format.type != GL_HALF_FLOAT_ARB, "image type must be half");
    error_check(format.format != GL_RGB && format.format != GL_RGBA && format.format != GL_LUMINANCE && format.format != GL_LUMINANCE_ALPHA, "image format must be rgb, rgba, luminance or luminance alpha");

    // TODO retype if float
    // TODO flip image

    short * pixels = (short *) image->pixels;

    Size stride = image_format_stride(format);
    Size size = format.size;
    int width = size.x;
    int height = size.y;

    Header header(width, height);
    header_add_properties(header, properties, property_count);

    FrameBuffer frame_buffer;

    switch (format.format)
    {
        case GL_LUMINANCE:
            header.channels().insert("Y", Channel(HALF)); frame_buffer.insert("Y", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
            break;

        case GL_LUMINANCE_ALPHA:
            header.channels().insert("Y", Channel(HALF)); frame_buffer.insert("Y", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
            header.channels().insert("A", Channel(HALF)); frame_buffer.insert("A", Slice(HALF, (char *) &pixels[1], stride.x, stride.y));
            break;

        case GL_RGB:
            header.channels().insert("R", Channel(HALF)); frame_buffer.insert("R", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
            header.channels().insert("G", Channel(HALF)); frame_buffer.insert("G", Slice(HALF, (char *) &pixels[1], stride.x, stride.y));
            header.channels().insert("B", Channel(HALF)); frame_buffer.insert("B", Slice(HALF, (char *) &pixels[2], stride.x, stride.y));
            break;

        case GL_RGBA:
            header.channels().insert("R", Channel(HALF)); frame_buffer.insert("R", Slice(HALF, (char *) &pixels[0], stride.x, stride.y));
            header.channels().insert("G", Channel(HALF)); frame_buffer.insert("G", Slice(HALF, (char *) &pixels[1], stride.x, stride.y));
            header.channels().insert("B", Channel(HALF)); frame_buffer.insert("B", Slice(HALF, (char *) &pixels[2], stride.x, stride.y));
            header.channels().insert("A", Channel(HALF)); frame_buffer.insert("A", Slice(HALF, (char *) &pixels[3], stride.x, stride.y));
            break;
    }

    OutputFile file(name, header);
    file.setFrameBuffer(frame_buffer);
    file.writePixels(height);
}
#if 0
void exr_save_with_properties_single(Image const * image, char const name[], Property const properties[], int property_count)
{
    Image_Format format = image->format;

    error_check(format.type != GL_HALF_FLOAT_ARB, "image type must be half");
    error_check(format.format != GL_RGBA, "image format must be rgba");

    // TODO retype if float
    // TODO flip image

    Rgba const * pixels = (Rgba const *) image->pixels;

    Size size = format.size;
    int width  = size.x;
    int height = size.y;

    Header header(width, height);
    header_add_properties(header, properties, property_count);
    RgbaChannels channels = type_to_channels(format.format);

    RgbaOutputFile file(name, width, height, channels);
    file.setFrameBuffer(pixels, 1, width);
    file.writePixels(height);
}
#endif

static void add_load_buffer(FrameBuffer & frame_buffer, char const name[], Image * image, int depth, int channel)
{
    Size stride = image_format_stride(image->format);
    int channel_size = image_type_to_size(image->format.type);
    char * pixels = (char *) image->pixels;

    frame_buffer.insert(name, Slice(HALF, &pixels[depth * stride.z + channel * channel_size], stride.x, stride.y));
}

static void add_save_buffer(Header & header, FrameBuffer & frame_buffer, char const name[], Image const * image, int depth, int channel)
{
    Size stride = image_format_stride(image->format);
    int channel_size = image_type_to_size(image->format.type);
    char * pixels = (char *) image->pixels;

    header.channels().insert(name, Channel(HALF));
    frame_buffer.insert(name, Slice(HALF, &pixels[depth * stride.z + channel * channel_size], stride.x, stride.y));
}

void exr_save_with_properties(Image const * image, char const name[], Property const properties[], int property_count, char const * layer_names[])
{
    if (image->format.size.z == 1)
    {
        exr_save_with_properties_single(image, name, properties, property_count);
        return;
    }

    Image_Format format = image->format;

    error_check(format.type != GL_HALF_FLOAT_ARB, "image type must be half");
    error_check(format.format != GL_RGBA && format.format != GL_LUMINANCE, "image format must be rgba");

    Size size = format.size;
    int width  = size.x;
    int height = size.y;
    int depth  = size.z;

    Size stride = image_format_stride(format);
    int channel_size = image_type_to_size(format.type);

    Header header(width, height);
    header_add_properties(header, properties, property_count);

    FrameBuffer frame_buffer;

    for (int i = 0; i != depth; ++ i)
    {
        char layer_key[256];
        char layer_name[256];
        char r_name[256];
        char g_name[256];
        char b_name[256];
        char a_name[256];

        if (layer_names)
            sprintf(layer_name, "%s", layer_names[i]);
        else
            sprintf(layer_name, "%d", i);

        sprintf(layer_key, "layer_name.%d", i);
        header.insert(layer_key, StringAttribute(layer_name));

        sprintf(r_name, "%s.R", layer_name);
        sprintf(g_name, "%s.G", layer_name);
        sprintf(b_name, "%s.B", layer_name);
        sprintf(a_name, "%s.A", layer_name);

        add_save_buffer(header, frame_buffer, r_name, (Image *) image, i, 0);
        add_save_buffer(header, frame_buffer, g_name, (Image *) image, i, 1);
        add_save_buffer(header, frame_buffer, b_name, (Image *) image, i, 2);
        add_save_buffer(header, frame_buffer, a_name, (Image *) image, i, 3);
    }

    OutputFile file(name, header);
    file.setFrameBuffer(frame_buffer);
    file.writePixels(height);
} 

void exr_save_multi(char const name[], Image const * exv_image, Image const * est_var_image, Image const * emp_var_image, Property const properties[], int property_count, float const radii[], int radius_count, char const kernel_type[])
{
    Image_Format format = exv_image->format;

    error_check(! image_format_equal(exv_image->format, est_var_image->format), "expected value and estimated variance image formats must be equal");
    error_check(! image_format_equal(exv_image->format, emp_var_image->format), "expected value and empirical variance image formats must be equal");
    error_check(format.type != GL_HALF_FLOAT_ARB, "image type must be half");
    error_check(format.format != GL_RGBA && format.format != GL_LUMINANCE, "image format must be rgba");
    error_check(format.size.z != radius_count, "radius count must much image depth");

    Size size = format.size;
    int width  = size.x;
    int height = size.y;
    int depth  = size.z;

    char radius_count_string[256];
    sprintf(radius_count_string, "%d", radius_count);

    Header header(width, height);
    header_add_properties(header, properties, property_count);
    header.insert("kernel_type",  StringAttribute(kernel_type));
    header.insert("radius_count", IntAttribute(radius_count));
    header.insert("RadiusCount", IntAttribute(radius_count));

    FrameBuffer frame_buffer;

    for (int i = 0; i != radius_count; ++ i)
    {
        char radius_key[256];
        char r_name[256];
        char g_name[256];
        char b_name[256];
        char a_name[256];

        sprintf(radius_key, "radius.%d", i);
        header.insert(radius_key, FloatAttribute(radii[i]));

        sprintf(r_name, "exv.%d.R", i);
        sprintf(g_name, "exv.%d.G", i);
        sprintf(b_name, "exv.%d.B", i);
        sprintf(a_name, "exv.%d.A", i);

        add_save_buffer(header, frame_buffer, r_name, exv_image, i, 0);
        add_save_buffer(header, frame_buffer, g_name, exv_image, i, 1);
        add_save_buffer(header, frame_buffer, b_name, exv_image, i, 2);
        add_save_buffer(header, frame_buffer, a_name, exv_image, i, 3);

        sprintf(r_name, "est_var.%d.R", i);
        sprintf(g_name, "est_var.%d.G", i);
        sprintf(b_name, "est_var.%d.B", i);
        sprintf(a_name, "est_var.%d.A", i);

        add_save_buffer(header, frame_buffer, r_name, est_var_image, i, 0);
        add_save_buffer(header, frame_buffer, g_name, est_var_image, i, 1);
        add_save_buffer(header, frame_buffer, b_name, est_var_image, i, 2);
        add_save_buffer(header, frame_buffer, a_name, est_var_image, i, 3);

        sprintf(r_name, "emp_var.%d.R", i);
        sprintf(g_name, "emp_var.%d.G", i);
        sprintf(b_name, "emp_var.%d.B", i);
        sprintf(a_name, "emp_var.%d.A", i);

        add_save_buffer(header, frame_buffer, r_name, emp_var_image, i, 0);
        add_save_buffer(header, frame_buffer, g_name, emp_var_image, i, 1);
        add_save_buffer(header, frame_buffer, b_name, emp_var_image, i, 2);
        add_save_buffer(header, frame_buffer, a_name, emp_var_image, i, 3);
    }

    OutputFile file(name, header);
    file.setFrameBuffer(frame_buffer);
    file.writePixels(height);
}

void exr_save_feature_buffer(char const name[],
    Image const * radiance_exv,
    Image const * radiance_var,
    Image const * weight_exv,
    Image const * weight_var,
    Image const * position_exv,
    Image const * position_var,
    Image const * normal_exv,
    Image const * normal_var,
    Property const properties[], int property_count)
{
    Image_Format color_format  = radiance_exv->format;
    Image_Format vector_format = position_exv->format;

    error_check(! image_format_equal(radiance_exv->format, radiance_var->format), "radiance exv and var formats must be equal");
    error_check(! image_format_equal(weight_exv->format,   weight_var->format),   "weight exv and var formats must be equal");
    error_check(! image_format_equal(radiance_exv->format, weight_exv->format),   "radiance and weight formats must be equal");
    error_check(! image_format_equal(position_exv->format, position_var->format), "position exv and var formats must be equal");
    error_check(! image_format_equal(normal_exv->format,   normal_var->format),   "normal exv and var formats must be equal");
    error_check(! image_format_equal(position_exv->format, normal_exv->format),   "position and normal formats must be equal");

    error_check(color_format.type   != GL_HALF_FLOAT_ARB, "color image type must be half");
    error_check(color_format.format != GL_RGBA, "color image format must be rgba or luminance");
    error_check(color_format.size.z != 1, "color layer depth must be 1");

    error_check(vector_format.type   != GL_HALF_FLOAT_ARB, "vector image type must be half");
    error_check(vector_format.format != GL_RGB, "vector image format must be rgb");
    error_check(vector_format.size.z != 1, "vector layer depth must be 1");

    Size size = color_format.size;
    int width  = size.x;
    int height = size.y;

    Header header(width, height);
    header_add_properties(header, properties, property_count);

    FrameBuffer frame_buffer;

    add_save_buffer(header, frame_buffer, "radiance.exv.R", radiance_exv, 0, 0);
    add_save_buffer(header, frame_buffer, "radiance.exv.G", radiance_exv, 0, 1);
    add_save_buffer(header, frame_buffer, "radiance.exv.B", radiance_exv, 0, 2);
    add_save_buffer(header, frame_buffer, "radiance.exv.A", radiance_exv, 0, 3);

    add_save_buffer(header, frame_buffer, "radiance.var.R", radiance_var, 0, 0);
    add_save_buffer(header, frame_buffer, "radiance.var.G", radiance_var, 0, 1);
    add_save_buffer(header, frame_buffer, "radiance.var.B", radiance_var, 0, 2);
    add_save_buffer(header, frame_buffer, "radiance.var.A", radiance_var, 0, 3);

    add_save_buffer(header, frame_buffer, "weight.exv.R",   weight_exv,   0, 0);
    add_save_buffer(header, frame_buffer, "weight.exv.G",   weight_exv,   0, 1);
    add_save_buffer(header, frame_buffer, "weight.exv.B",   weight_exv,   0, 2);
    add_save_buffer(header, frame_buffer, "weight.exv.A",   weight_exv,   0, 3);

    add_save_buffer(header, frame_buffer, "weight.var.R",   weight_var,   0, 0);
    add_save_buffer(header, frame_buffer, "weight.var.G",   weight_var,   0, 1);
    add_save_buffer(header, frame_buffer, "weight.var.B",   weight_var,   0, 2);
    add_save_buffer(header, frame_buffer, "weight.var.A",   weight_var,   0, 3);

    add_save_buffer(header, frame_buffer, "position.exv.X", position_exv, 0, 0);
    add_save_buffer(header, frame_buffer, "position.exv.Y", position_exv, 0, 1);
    add_save_buffer(header, frame_buffer, "position.exv.Z", position_exv, 0, 2);

    add_save_buffer(header, frame_buffer, "position.var.X", position_var, 0, 0);
    add_save_buffer(header, frame_buffer, "position.var.Y", position_var, 0, 1);
    add_save_buffer(header, frame_buffer, "position.var.Z", position_var, 0, 2);

    add_save_buffer(header, frame_buffer, "normal.exv.X",   normal_exv,   0, 0);
    add_save_buffer(header, frame_buffer, "normal.exv.Y",   normal_exv,   0, 1);
    add_save_buffer(header, frame_buffer, "normal.exv.Z",   normal_exv,   0, 2);

    add_save_buffer(header, frame_buffer, "normal.var.X",   normal_var,   0, 0);
    add_save_buffer(header, frame_buffer, "normal.var.Y",   normal_var,   0, 1);
    add_save_buffer(header, frame_buffer, "normal.var.Z",   normal_var,   0, 2);

    OutputFile file(name, header);
    file.setFrameBuffer(frame_buffer);
    file.writePixels(height);
}

void exr_load_multi(char const name[], Image ** exv_image, Image ** est_var_image, Image ** emp_var_image, float ** radii, int * radius_count, char kernel_type[])
{
    InputFile file(name);
    Header const & header = file.header();
    // TODO check_header_wavelet();

    Box2i box = header.dataWindow();
    int width  = box.max.x - box.min.x + 1;
    int height = box.max.y - box.min.y + 1;

    IntAttribute const * radius_count_attribute = header.findTypedAttribute<IntAttribute>("radius_count");
    * radius_count = radius_count_attribute->value();

    * radii = malloc_array(float, * radius_count);
    for (int i = 0; i != * radius_count; ++ i)
    {
        char buffer[256];
        sprintf(buffer, "radius.%d", i);

        FloatAttribute const * radius_attribute = header.findTypedAttribute<FloatAttribute>(buffer);
        (* radii)[i] = radius_attribute->value();
    }

    Size size = {width, height, * radius_count};

    Image_Format format = {GL_HALF_FLOAT, GL_RGBA, size};
    * exv_image = image_new(format);
    * est_var_image = image_new(format);
    * emp_var_image = image_new(format);

    FrameBuffer frame_buffer;
    for (int i = 0; i != * radius_count; ++ i)
    {
        char r_name[256];
        char g_name[256];
        char b_name[256];
        char a_name[256];

        sprintf(r_name, "exv.%d.R", i);
        sprintf(g_name, "exv.%d.G", i);
        sprintf(b_name, "exv.%d.B", i);
        sprintf(a_name, "exv.%d.A", i);

        add_load_buffer(frame_buffer, r_name, * exv_image, i, 0);
        add_load_buffer(frame_buffer, g_name, * exv_image, i, 1);
        add_load_buffer(frame_buffer, b_name, * exv_image, i, 2);
        add_load_buffer(frame_buffer, a_name, * exv_image, i, 3);

        sprintf(r_name, "est_var.%d.R", i);
        sprintf(g_name, "est_var.%d.G", i);
        sprintf(b_name, "est_var.%d.B", i);
        sprintf(a_name, "est_var.%d.A", i);

        add_load_buffer(frame_buffer, r_name, * est_var_image, i, 0);
        add_load_buffer(frame_buffer, g_name, * est_var_image, i, 1);
        add_load_buffer(frame_buffer, b_name, * est_var_image, i, 2);
        add_load_buffer(frame_buffer, a_name, * est_var_image, i, 3);

        sprintf(r_name, "emp_var.%d.R", i);
        sprintf(g_name, "emp_var.%d.G", i);
        sprintf(b_name, "emp_var.%d.B", i);
        sprintf(a_name, "emp_var.%d.A", i);

        add_load_buffer(frame_buffer, r_name, * emp_var_image, i, 0);
        add_load_buffer(frame_buffer, g_name, * emp_var_image, i, 1);
        add_load_buffer(frame_buffer, b_name, * emp_var_image, i, 2);
        add_load_buffer(frame_buffer, a_name, * emp_var_image, i, 3);
    }

    file.setFrameBuffer(frame_buffer);
    file.readPixels(box.min.y, box.max.y);
}

void exr_load_feature_buffer(char const name[],
    Image ** radiance_exv,
    Image ** radiance_var,
    Image ** weight_exv,
    Image ** weight_var,
    Image ** position_exv,
    Image ** position_var,
    Image ** normal_exv,
    Image ** normal_var)
{
    InputFile file(name);
    Header const & header = file.header();
    // TODO check_header_feature_buffer();

    Box2i box = header.dataWindow();
    int width  = box.max.x - box.min.x + 1;
    int height = box.max.y - box.min.y + 1;

    Size size = {width, height, 1};

    Image_Format color_format  = {GL_HALF_FLOAT, GL_RGBA, size};

    * radiance_exv = image_new(color_format);
    * radiance_var = image_new(color_format);
    * weight_exv   = image_new(color_format);
    * weight_var   = image_new(color_format);

    Image_Format vector_format = {GL_HALF_FLOAT, GL_RGB,  size};

    * position_exv = image_new(vector_format);
    * position_var = image_new(vector_format);
    * normal_exv   = image_new(vector_format);
    * normal_var   = image_new(vector_format);

    FrameBuffer frame_buffer;

    add_load_buffer(frame_buffer, "radiance.exv.R", * radiance_exv, 0, 0);
    add_load_buffer(frame_buffer, "radiance.exv.G", * radiance_exv, 0, 1);
    add_load_buffer(frame_buffer, "radiance.exv.B", * radiance_exv, 0, 2);
    add_load_buffer(frame_buffer, "radiance.exv.A", * radiance_exv, 0, 3);

    add_load_buffer(frame_buffer, "radiance.var.R", * radiance_var, 0, 0);
    add_load_buffer(frame_buffer, "radiance.var.G", * radiance_var, 0, 1);
    add_load_buffer(frame_buffer, "radiance.var.B", * radiance_var, 0, 2);
    add_load_buffer(frame_buffer, "radiance.var.A", * radiance_var, 0, 3);

    add_load_buffer(frame_buffer, "weight.exv.R",   * weight_exv, 0, 0);
    add_load_buffer(frame_buffer, "weight.exv.G",   * weight_exv, 0, 1);
    add_load_buffer(frame_buffer, "weight.exv.B",   * weight_exv, 0, 2);
    add_load_buffer(frame_buffer, "weight.exv.A",   * weight_exv, 0, 3);

    add_load_buffer(frame_buffer, "weight.var.R",   * weight_var, 0, 0);
    add_load_buffer(frame_buffer, "weight.var.G",   * weight_var, 0, 1);
    add_load_buffer(frame_buffer, "weight.var.B",   * weight_var, 0, 2);
    add_load_buffer(frame_buffer, "weight.var.A",   * weight_var, 0, 3);

    add_load_buffer(frame_buffer, "position.exv.X", * position_exv, 0, 0);
    add_load_buffer(frame_buffer, "position.exv.Y", * position_exv, 0, 1);
    add_load_buffer(frame_buffer, "position.exv.Z", * position_exv, 0, 2);

    add_load_buffer(frame_buffer, "position.var.X", * position_var, 0, 0);
    add_load_buffer(frame_buffer, "position.var.Y", * position_var, 0, 1);
    add_load_buffer(frame_buffer, "position.var.Z", * position_var, 0, 2);

    add_load_buffer(frame_buffer, "normal.exv.X",   * normal_exv, 0, 0);
    add_load_buffer(frame_buffer, "normal.exv.Y",   * normal_exv, 0, 1);
    add_load_buffer(frame_buffer, "normal.exv.Z",   * normal_exv, 0, 2);

    add_load_buffer(frame_buffer, "normal.var.X",   * normal_var, 0, 0);
    add_load_buffer(frame_buffer, "normal.var.Y",   * normal_var, 0, 1);
    add_load_buffer(frame_buffer, "normal.var.Z",   * normal_var, 0, 2);

    file.setFrameBuffer(frame_buffer);
    file.readPixels(box.min.y, box.max.y);
}

static int channel_name_to_index(string const & channel_name)
{
    if (channel_name.empty())
        return -1;

    switch (channel_name[channel_name.length() - 1])
    {
        case 'R': return 0;
        case 'G': return 1;
        case 'B': return 2;
        case 'A': return 3;
    }

    return -1;
}

static void setup_frame_buffer_for_load(FrameBuffer & frame_buffer, ChannelList const & channels, Image * image)
{
    unsigned char * pixels = (unsigned char *) image->pixels;

    Image_Format format = image->format;
    Size stride = image_format_stride(format);
//    int channel_size = image_type_to_size(format.type);

    set<string> layer_names;
    channels.layers(layer_names);

    int k = 0;
    for (set<string>::const_iterator i = layer_names.begin(); i != layer_names.end(); ++ i)
    {
//        cout << "layer " << *i << endl;

        ChannelList::ConstIterator layer_begin, layer_end;
        channels.channelsInLayer(*i, layer_begin, layer_end);

        for (ChannelList::ConstIterator j = layer_begin; j != layer_end; ++ j)
        {
            int channel_index = channel_name_to_index(j.name());
//            cout << "channel " << j.name() << ", index = " << channel_index << endl;

            add_load_buffer(frame_buffer, j.name(), image, k, channel_index);
        }

        ++ k;
    }
}

static void setup_frame_buffer_for_load_layer(FrameBuffer & frame_buffer, ChannelList const & channels, char const layer_name[], Image * image)
{
    unsigned char * pixels = (unsigned char *) image->pixels;

    Image_Format format = image->format;
    Size stride = image_format_stride(format);
    //int channel_size = image_type_to_size(format.type);

    ChannelList::ConstIterator begin, end, i;
    channels.channelsInLayer(layer_name, begin, end);

    int k = 0;
    for (i = begin; i != end; ++ i)
    {
        int channel_index = channel_name_to_index(i.name());
//        cout << "channel " << i.name() << ", index = " << channel_index << endl;

        add_load_buffer(frame_buffer, i.name(), image, 0, channel_index);

        ++ k;
    }
}

Image * exr_load(char const name[])
{
    InputFile file(name);
    Header const & header = file.header();

    ChannelList const & channels = header.channels();
    set<string> layer_names;
    channels.layers(layer_names);

    if (layer_names.size() == 0)
        return exr_load_single(name);

    Box2i box = header.dataWindow();
    int width  = box.max.x - box.min.x + 1;
    int height = box.max.y - box.min.y + 1;
    int depth = layer_names.size();
    Size size = {width, height, depth};

    Image_Format format = {GL_HALF_FLOAT, GL_RGBA, size};
    Image * image = image_new(format);
    unsigned short * pixels = (unsigned short *) image->pixels;

    FrameBuffer frame_buffer;
    setup_frame_buffer_for_load(frame_buffer, channels, image);

    file.setFrameBuffer(frame_buffer);
    file.readPixels(box.min.y, box.max.y);

    image_flip(image);

    return image;
}

Image * exr_load_layer(char const name[], char const layer_name[])
{
    InputFile file(name);
    Header const & header = file.header();
    ChannelList const & channels = header.channels();

    Box2i box = header.dataWindow();
    int width  = box.max.x - box.min.x + 1;
    int height = box.max.y - box.min.y + 1;
    Size size = {width, height, 1};

    Image_Format format = {GL_HALF_FLOAT, GL_RGBA, size};
    Image * image = image_new(format);

    FrameBuffer frame_buffer;
    setup_frame_buffer_for_load_layer(frame_buffer, channels, layer_name, image);

    file.setFrameBuffer(frame_buffer);
    file.readPixels(box.min.y, box.max.y);

    image_flip(image);

    return image;
}

Property * exr_image_properties(char const name[], unsigned * count)
{
    InputFile file(name);
    Header const & header = file.header();

    set<string> layer_names;
    header.channels().layers(layer_names);
    
    if (layer_names.size() == 0)
    {
        * count = 0;
        return NULL;
    }

    * count = layer_names.size();

    Property * properties = malloc_array(Property, * count);
    int j = 0;

    for (set<string>::const_iterator i = layer_names.begin(); i != layer_names.end(); ++ i)
    {
        char layer_id[256];
        sprintf(layer_id, "layer_name.%d", j);

        properties[j].key   = strdup(layer_id);
        properties[j].value = strdup(i->c_str());

        ++ j;
    }

    return properties;
}

#else

Image * exr_load(char const name[]) {return NULL;}
void exr_save_with_properties(Image const * image, char const name[], Property const properties[], int property_count) {}

#endif

void exr_save(Image const * image, char const name[])
{
    exr_save_with_properties(image, name, NULL, 0, NULL);
}
