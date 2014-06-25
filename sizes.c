#include "string.h"
#include "utils.h"
#include "viewport.h"

static struct
{
    float aspect_ratio;
    char const * name;
}
aspect_ratios[] =
{
    {1.0,        "Square"},
    {4.0 / 3.0,  "TV"},
    {3.0 / 2.0,  "Photo"},
    {1.6,        "16:10"},
    {1.618,      "Golden Ratio"},
    {16.0 / 9.0, "HDTV"},
    {1.85,       "Widescreen"},
    {2.39,       "Anamorphic"} // cineascope : 21/9 = 2.33
};

static struct
{
    Resolution resolution;
    char const * name;
}
resolutions[] = 
{
    /* 3:2, photo */
    {{ 720,  480}, "NTSC"},
    {{3008, 2000}, "camera 6mp"},

    /* 4:3, TV */
    {{ 640,  480}, "VGA"},
    {{ 800,  600}, "SVGA"},
    {{1024,  768}, "XGA"},
    {{1280,  960}, "SXGA-"},
    /* 1400x1050 */
    /* 1440x1080 (HDTV) */
    {{1600, 1200}, "UXGA"},

    /* 5:4 */
    {{ 720,  576}, "PAL"},
    {{1280, 1024}, "SXGA"},

    /* 16:9, HDTV */
    {{ 640,  360}, "QHD"},
    {{1024,  576}, "WSVGA"},
    {{1280,  720}, "WXGA minimal"}, /* (HDTV)*/
    {{1920, 1080}, "HDTV 1080p"},

    /* 16:10 */
    {{1680, 1050}, "WSXGA+"},
    {{1920, 1200}, "WUXGA"},
    {{2560, 1600}, "WQXGA"},

    /* 1.18:1 */
    /* 1280x1080 (HDTV) */

    /* 2.39:1 */
    {{4096, 1714}, "anamorphic 4k"},
    {{2048,  858}, "anamorphic 2k"},

    /* 1.85:1 */
    {{3996, 2160}, "widescreen 4k"},
    {{1998, 1080}, "widescreen 2k"}
};

int aspect_ratio_from_name(char const name[], float * aspect_ratio)
{
    int i;

    for (i = 0; i != array_count(aspect_ratios); ++ i)
    {
        if (streq(name, aspect_ratios[i].name))
        {
            * aspect_ratio = aspect_ratios[i].aspect_ratio;
            return 1;
        }
    }

    return 0;
}

int resolution_from_name(char const name[], Resolution * resolution)
{
    int i;

    for (i = 0; i != array_count(resolutions); ++ i)
    {
        if (streq(name, resolutions[i].name))
        {
            * resolution = resolutions[i].resolution;
            return 1;
        }
    }

    return 0;
}

