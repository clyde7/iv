#include <stdlib.h>

#include "file.h"
#include "file_image.h"

char const * MPO_MIME = "image/x-mpo";

Image * mpo_load(char const filename[])
{
    char command[512], name[L_tmpnam];
    Image * images[2];

    file_temporary_name(name);

    sprintf(command, "exiftool -trailer:all= %s -o %s", filename, name);
    system(command);
    images[0] = jpeg_load(fopen(name, "rb"));

    remove(name);
    file_temporary_name(name);

    sprintf(command, "exiftool %s -mpimage2 -b > %s", filename, name);
    system(command);
    images[1] = jpeg_load(fopen(name, "rb"));

    Image * image = image_stack((Image const **) images, 2);
    image_destroy(images[0]);
    image_destroy(images[1]);

    char parallax_name[L_tmpnam];
    file_temporary_name(parallax_name);

    sprintf(command, "exiftool -S -Parallax %s > %s", name, parallax_name);
    system(command);

    unsigned length;
    char * buffer = file_read_text(parallax_name, &length);

    float parallax;
    sscanf(buffer, "Parallax: %f", &parallax);
    printf("parallax is %f\n", parallax);

    float disparity = (parallax / 100) * image->format.size.x;
    printf("disparity  is %f\n", disparity);

    // TODO now pass the parallax

    remove(name);
    remove(parallax_name);

    return image;
}
