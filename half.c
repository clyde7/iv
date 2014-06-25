#include <stdio.h>

#include "half.h"

static int initialized;
static unsigned mantissa[2048], exponent[64], offset[64];
static unsigned short base[512];
static unsigned char shift[512];

static unsigned convert_mantissa(unsigned i)
{
    unsigned m = i << 13;
    unsigned e = 0;

    while (! (m & 0x00800000))
    {
        m <<= 1;
        e -= 0x00800000;
    }

    m &= ~0x00800000;
    e += 0x38800000;

    return m | e;
}

void half_initialize(void)
{
    if (initialized)
        return;

    unsigned int i;

    // half to float

    mantissa[0] = 0;
    for (i =    1; i <= 1023; ++ i) mantissa[i] = convert_mantissa(i);
    for (i = 1024; i <= 2047; ++ i) mantissa[i] = 0x38000000 + ((i - 1024) << 13);

    exponent[ 0] = 0;
    exponent[31] = 0x47800000;
    exponent[32] = 0x80000000;
    exponent[63] = 0xC7800000;
    for (i =  1; i <= 30; ++ i) exponent[i] = i << 23;
    for (i = 33; i <= 62; ++ i) exponent[i] = 0x80000000 + ((i - 32) << 23);

    offset[ 0] = 0;
    offset[32] = 0;
    for (i =  1; i <= 31; ++ i) offset[i] = 1024;
    for (i = 33; i <= 63; ++ i) offset[i] = 1024;

    // float to half

    for (i = 0; i < 256; ++ i)
    {
        int e = i - 127;

        if (e < -24)
        {    // Very small numbers map to zero basetable[i|0x000]=0x0000;
            base[i|0x100]=0x8000;
            shift[i|0x000]=24;
            shift[i|0x100]=24;
        }
        else if (e < -14)
        {   // Small numbers map to denorms
            base[i|0x000] = (0x0400>>(18-e));
            base[i|0x100] = (0x0400>>(18-e)) | 0x8000;
            shift[i|0x000] = -e-1;
            shift[i|0x100] = -e-1;
        }
        else if (e <= 15)
        {   // Normal numbers just lose precision
            base[i|0x000] = ((e+15)<<10);
            base[i|0x100] = ((e+15)<<10) | 0x8000;
            shift[i|0x000] = 13;
            shift[i|0x100] = 13;
        }
        else if (e < 128)
        {   // Large numbers map to Infinity
            base[i|0x000] = 0x7C00;
            base[i|0x100] = 0xFC00;
            shift[i|0x000] = 24;
            shift[i|0x100] = 24;
        }
        else
        {   // Infinity and NaN's stay Infinity and NaN's
            base[i|0x000] = 0x7C00;
            base[i|0x100] = 0xFC00;
            shift[i|0x000] = 13;
            shift[i|0x100] = 13;
        }
    }

    initialized = 1;
}

float half_to_float_slow(unsigned short h)
{
    // for normal numbers only
    unsigned f = ((h & 0x8000) << 16) | (((h & 0x7c00) + 0x1C000) << 13) | ((h & 0x03FF) << 13);
    return * (float *) &f;
}

float half_to_float(unsigned short h)
{
    unsigned short e = h >> 10;
    unsigned f = exponent[e] + mantissa[(h & 0x3ff) + offset[e]];

    return * (float *) &f;
}

unsigned short half_from_float_slow(float f_)
{
    unsigned f = * (unsigned *) &f_;
    return ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);
}

unsigned short half_from_float(float f_)
{
    unsigned f = * (unsigned *) &f_;
    unsigned e = (f >> 23) & 0x1ff;

    return base[e] + ((f & 0x007fffff) >> shift[e]);
}

void half_print(unsigned short h)
{
    unsigned s = h >> 15;
    unsigned short e = (h >> 10) & 0x1F;
    unsigned short m = h & 0x3FF;

    char sign = s ? '-' : '+';

    if (e == 0)
    {
        if (m == 0)
            printf("zero: %c0", sign);
        else
            printf("subnormal: %c%f 2^%d", sign, m / 1024.0, -14);
    }
    else if (e == 31)
    {
        if (m == 0)
            printf("infinity: %cinf", sign);
        else
            printf("not a number: %cNaN", sign);
    }
    else 
        printf("normal: %c%f 2^%d", sign, 1.0 + m / 1024.0, e - 15);
}

void identity(float f)
{
    printf("slow expected = %f, actual = %f\n", f, half_to_float_slow(half_from_float_slow(f)));
    printf("fast expected = %f, actual = %f\n", f, half_to_float(half_from_float(f)));
}

void half_test(void)
{
//    identity(0); // fails
    identity(1);
    identity(42);
//    identity(1.0/0.0); // fails
}

