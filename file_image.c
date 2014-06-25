#include <stdlib.h>

#include "error.h"
#include "file.h"
#include "file_image.h"
#include "string.h"
#include "utils.h"

#ifdef CYGWIN
static char const CYGPATH[] = "cygpath -w";
#endif

typedef struct {char const * mime, * extension;} Entry;

static Entry mime_to_extensions[] =
{
    {"image/xexr", "exr"},
    {"image/png", "png"},
    {"image/jpeg", "jpg"}
};
static int const type_count = array_count(mime_to_extensions);

static char const * mime_to_extension(char const mime[])
{
    for (int i = 0; i != type_count; ++ i)
    {
        Entry const * entry = &mime_to_extensions[i];
        if (streq(entry->mime, mime))
        {
            return entry->extension;
        }
    }
    
    return NULL;
}

static void name_from_pattern(char const basename[], char const mime_type[], char name[])
{
    char const * extension = mime_to_extension(mime_type);

    char pattern[256];
    sprintf(pattern, "%s-%%04d.%s", basename, extension);
    file_unique_name(name, pattern);
}

int image_save(Image const * image, Property const properties[], int property_count, char const path[], char const mime_type[])
{
#ifdef EXR
    if (streq(mime_type, EXR_MIME))
    {
        exr_save_with_properties(image, path, properties, property_count, NULL);
        return 1;
    }
#endif

    if (streq(mime_type, PNG_MIME))
    {
        FILE * file = fopen(path, "wb");
        return png_save_with_properties(image, file, properties, property_count);
    }

    if (streq(mime_type, JPEG_MIME))
    {
        jpeg_save(image, fopen(path, "wb"));
        return 1;
    }

    char const * temp_name = tmpnam(NULL);
    int success = png_save(image, fopen(temp_name, "wb"));
    if (! success)
        return 0;

    char command[256];
#ifdef CYGWIN
    sprintf(command, "convert `%s %s` `%s %s`", CYGPATH, temp_name, CYGPATH, path);
#else
    sprintf(command, "convert %s %s", temp_name, path);
#endif

    printf("executing \"%s\"\n", command);
    int status = system(command);
    error_check_arg(status != EXIT_SUCCESS, "unsupported image format \"%s\"", mime_type);

    if (remove(temp_name) != 0)
    {
        perror("failed to delete");
        success = 0;
    }

    return success;
}

int image_save_basename(Image const * image, Property const properties[], int property_count, char const basename[], char const mime_type[])
{
    char path[256];
    name_from_pattern(basename, mime_type, path);

    return image_save(image, properties, property_count, path, mime_type);
}

