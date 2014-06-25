#ifndef SIZE_H
#define SIZE_H

typedef enum {BORDER_BLACK, BORDER_CLAMP, BORDER_WRAP, BORDER_MIRROR} Border;

typedef struct {int x, y, z;} Size;

extern Size const MAX_SIZE;
extern Size const MIN_SIZE;

Size size_wrap(int, int, int);
void size_print(Size);
Size size_stride(Size);
Size size_add(Size, Size);
Size size_scale(Size, Size);
int  size_index(Size, int, int, int);
int  size_index_border(Size, int i, int j, int k, Border);
int  size_total(Size);
int  size_equal(Size, Size);
int  size_volume(Size);
int  size_empty(Size);
Size size_min(Size, Size);
Size size_max(Size, Size);

int  tuple_parser(void const *, char const string[], void * target);
int  size_parser(void const *, char const string[], void * target);
int  size_printer(void const *, void const * source, char string[]);

#endif
