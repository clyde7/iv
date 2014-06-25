#ifndef HALF_H
#define HALF_H

void           half_print(unsigned short);
void           half_initialize(void);
float          half_to_float(unsigned short);
float          half_to_float_slow(unsigned short);
unsigned short half_from_float(float);
unsigned short half_from_float_slow(float);

void           half_test(void);

#endif
