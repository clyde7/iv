#include <stdio.h>
#include <stdlib.h>

#include "file_image.h"
#include "image_process.h"

int main(int argc, char * argv[])
{
    Image * image = image_grey_gradient(GL_UNSIGNED_SHORT);
    png_save(image, fopen("test-gray.png", "wb"));

    return EXIT_SUCCESS;
}
