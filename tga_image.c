
typedef enum
{
    NO_IMAGE = 0,
    UNCOMPRESSED_COLOR_MAPPED = 1,
    UNCOMPRESSED_TRUE_COLOR = 2,
    UNCOMPRESSED_GRAY = 3,
    RUN_LENGTH_COLOR_MAPPED = 9,
    RUN_LENGTH_TRUE_COLOR = 10,
    RUN_LENGTH_GRAY = 11
}
Image_Type;

typedef struct
{
    unsigned short offset;
    unsigned short length;
    unsigned char depth;
}
Color_Map_Spec;

typedef struct
{
    unsigned short x, y, w, h;
    unsigned char pixel_depth;
    unsigned char bits; // 00vhaaaa
}
Image_Spec;

typedef struct
{
    unsigned char ID_length;
    unsigned char color_map_type;
    unsigned char image_type;
    Color_Map_Spec color_map_spec;
    Image_Spec image_spec;
}
Header;

char const * TGA1_MIME = "image/x-targa";
char const * TGA2_MIME = "image/x-tga";

