#include <float.h>
#include <stdlib.h>

#include "color.h"
#include "error.h"
#include "string.h"
#include "utils.h"

#define BINARY_SEARCH

static struct SVG_Color
{
    unsigned long value;
    char const * name;
}
svg_colors[] =
{
    {0xf0f8ff, "aliceblue"},
    {0xfaebd7, "antiquewhite"},
    {0x00ffff, "aqua"},
    {0x7fffd4, "aquamarine"},
    {0xf0ffff, "azure"},
    {0xf5f5dc, "beige"},
    {0xffe4c4, "bisque"},
    {0x000000, "black"},
    {0xffebcd, "blanchedalmond"},
    {0x0000ff, "blue"},
    {0x8a2be2, "blueviolet"},
    {0xa52a2a, "brown"},
    {0xdeb887, "burlywood"},
    {0x5f9ea0, "cadetblue"},
    {0x7fff00, "chartreuse"},
    {0xd2691e, "chocolate"},
    {0xff7f50, "coral"},
    {0x6495ed, "cornflowerblue"},
    {0xfff8dc, "cornsilk"},
    {0xdc143c, "crimson"},
    {0x00ffff, "cyan"},
    {0x00008b, "darkblue"},
    {0x008b8b, "darkcyan"},
    {0xb8860b, "darkgoldenrod"},
    {0xa9a9a9, "darkgray"},
    {0x006400, "darkgreen"},
    {0xa9a9a9, "darkgrey"},
    {0xbdb76b, "darkkhaki"},
    {0x8b008b, "darkmagenta"},
    {0x556b2f, "darkolivegreen"},
    {0xff8c00, "darkorange"},
    {0x9932cc, "darkorchid"},
    {0x8b0000, "darkred"},
    {0xe9967a, "darksalmon"},
    {0x8fbc8f, "darkseagreen"},
    {0x483d8b, "darkslateblue"},
    {0x2f4f4f, "darkslategray"},
    {0x2f4f4f, "darkslategrey"},
    {0x00ced1, "darkturquoise"},
    {0x9400d3, "darkviolet"},
    {0xff1493, "deeppink"},
    {0x00bfff, "deepskyblue"},
    {0x696969, "dimgray"},
    {0x696969, "dimgrey"},
    {0x1e90ff, "dodgerblue"},
    {0xb22222, "firebrick"},
    {0xfffaf0, "floralwhite"},
    {0x228b22, "forestgreen"},
    {0xff00ff, "fuchsia"},
    {0xdcdcdc, "gainsboro"},
    {0xf8f8ff, "ghostwhite"},
    {0xffd700, "gold"},
    {0xdaa520, "goldenrod"},
    {0x808080, "gray"},
    {0x008000, "green"},
    {0xadff2f, "greenyellow"},
    {0x808080, "grey"},
    {0xf0fff0, "honeydew"},
    {0xff69b4, "hotpink"},
    {0xcd5c5c, "indianred"},
    {0x4b0082, "indigo"},
    {0xfffff0, "ivory"},
    {0xf0e68c, "khaki"},
    {0xe6e6fa, "lavender"},
    {0xfff0f5, "lavenderblush"},
    {0x7cfc00, "lawngreen"},
    {0xfffacd, "lemonchiffon"},
    {0xadd8e6, "lightblue"},
    {0xf08080, "lightcoral"},
    {0xe0ffff, "lightcyan"},
    {0xfafad2, "lightgoldenrodyellow"},
    {0xd3d3d3, "lightgray"},
    {0x90ee90, "lightgreen"},
    {0xd3d3d3, "lightgrey"},
    {0xffb6c1, "lightpink"},
    {0xffa07a, "lightsalmon"},
    {0x20b2aa, "lightseagreen"},
    {0x87cefa, "lightskyblue"},
    {0x778899, "lightslategray"},
    {0x778899, "lightslategrey"},
    {0xb0c4de, "lightsteelblue"},
    {0xffffe0, "lightyellow"},
    {0x00ff00, "lime"},
    {0x32cd32, "limegreen"},
    {0xfaf0e6, "linen"},
    {0xff00ff, "magenta"},
    {0x800000, "maroon"},
    {0x66cdaa, "mediumaquamarine"},
    {0x0000cd, "mediumblue"},
    {0xba55d3, "mediumorchid"},
    {0x9370db, "mediumpurple"},
    {0x3cb371, "mediumseagreen"},
    {0x7b68ee, "mediumslateblue"},
    {0x00fa9a, "mediumspringgreen"},
    {0x48d1cc, "mediumturquoise"},
    {0xc71585, "mediumvioletred"},
    {0x191970, "midnightblue"},
    {0xf5fffa, "mintcream"},
    {0xffe4e1, "mistyrose"},
    {0xffe4b5, "moccasin"},
    {0xffdead, "navajowhite"},
    {0x000080, "navy"},
    {0xfdf5e6, "oldlace"},
    {0x808000, "olive"},
    {0x6b8e23, "olivedrab"},
    {0xffa500, "orange"},
    {0xff4500, "orangered"},
    {0xda70d6, "orchid"},
    {0xeee8aa, "palegoldenrod"},
    {0x98fb98, "palegreen"},
    {0xafeeee, "paleturquoise"},
    {0xdb7093, "palevioletred"},
    {0xffefd5, "papayawhip"},
    {0xffdab9, "peachpuff"},
    {0xcd853f, "peru"},
    {0xffc0cb, "pink"},
    {0xdda0dd, "plum"},
    {0xb0e0e6, "powderblue"},
    {0x800080, "purple"},
    {0xff0000, "red"},
    {0xbc8f8f, "rosybrown"},
    {0x4169e1, "royalblue"},
    {0x8b4513, "saddlebrown"},
    {0xfa8072, "salmon"},
    {0xf4a460, "sandybrown"},
    {0x2e8b57, "seagreen"},
    {0xfff5ee, "seashell"},
    {0xa0522d, "sienna"},
    {0xc0c0c0, "silver"},
    {0x87ceeb, "skyblue"},
    {0x6a5acd, "slateblue"},
    {0x708090, "slategray"},
    {0x708090, "slategrey"},
    {0xfffafa, "snow"},
    {0x00ff7f, "springgreen"},
    {0x4682b4, "steelblue"},
    {0xd2b48c, "tan"},
    {0x008080, "teal"},
    {0xd8bfd8, "thistle"},
    {0xff6347, "tomato"},
    {0x40e0d0, "turquoise"},
    {0xee82ee, "violet"},
    {0xf5deb3, "wheat"},
    {0xffffff, "white"},
    {0xf5f5f5, "whitesmoke"},
    {0xffff00, "yellow"},
    {0x9acd32, "yellowgreen"}
};

#ifdef BINARY_SEARCH

static int compare(void const * a, void const * b)
{
    return strcmp(((struct SVG_Color *) a)->name, ((struct SVG_Color *) b)->name);
}

int color_from_svg_name(char const name[], Color * color)
{
    struct SVG_Color key = {42, (char *) name};
    int const count = array_count(svg_colors);
    struct SVG_Color * entry = (struct SVG_Color *) bsearch(&key, svg_colors, count, sizeof(svg_colors[0]), compare);
    if (! entry)
        return 0;

    * color = color_from_value(entry->value);
    return 1;
}

#else

int color_from_svg_name(char const name[], Color * color)
{
    int i;

    for (i = 0; i != array_count(svg_colors); ++ i)
    {
        if (streq(name, svg_colors[i].name))
        {
            * color = color_from_value(svg_colors[i].value);
            return 1;
        }
    }

    return 0;
}

#endif

char const * color_to_svg_name(Color reference)
{
    int const count = array_count(svg_colors);

    float min_distance = FLT_MAX;
    int min_index = -1;

    for (int i = 0; i != count; ++ i)
    {
        Color color = color_from_value(svg_colors[i].value);
        float distance = color_distance(color, reference);
        if (distance > min_distance)
            continue;

        min_distance = distance;
        min_index = i;
    }

    return svg_colors[min_index].name;
}

Color color_from_name(char const name[])
{
    Color color;
    error_check(! color_from_svg_name(name, &color), "bad color name");        

    return color;
}
