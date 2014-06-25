#include <stdio.h>

#include "error.h"
#include "pixel_map.h"
#include "print.h"

void pixel_map_apply(Image const * pixel_map)
{
    error_check(pixel_map->format.size.y > 4, "bad pixel map size");
    error_check(pixel_map->format.format != GL_LUMINANCE, "bad pixel map format");
    error_check(pixel_map->format.type != GL_FLOAT, "bad pixel map type");

    Size size = pixel_map->format.size;
    float const * pixels = (float const *) pixel_map->pixels;

    glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, size.x, pixels + size_index(size, 0, 0, 0));
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, size.x, pixels + size_index(size, 0, 1, 0));
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, size.x, pixels + size_index(size, 0, 2, 0));
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, size.x, pixels + size_index(size, 0, 3, 0));
}

#define GAMMA_TABLE_SIZE (1024)

void pixel_map_correct_gamma(float gamma_)
{
    static float table[GAMMA_TABLE_SIZE];
    static float alpha = 1.0;
    static float gamma = 1.0;

    // XXX why does this not work?
//    if (gamma_ == gamma)
//        return;

    static GLint table_size;
    if (! table_size)
    {
        GLint max_pixel_map_table;
        glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, &max_pixel_map_table);
        table_size = imin(max_pixel_map_table, GAMMA_TABLE_SIZE);

        if (table_size != GAMMA_TABLE_SIZE)
            printf(ANSI_RED "warning" ANSI_RESET ": reducing gamma table size from %d to %d entries\n", GAMMA_TABLE_SIZE, table_size);
    }

    gamma = gamma_;

    for (int i = 0; i != table_size; ++ i)
    {
        table[i] = pow((float) i / (table_size - 1), 1.0 / gamma);
    }

    glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, table_size, table);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, table_size, table);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, table_size, table);
    glPixelMapfv(GL_PIXEL_MAP_A_TO_A, 1, &alpha);
}

void pixel_map_reset(void)
{
    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
}
