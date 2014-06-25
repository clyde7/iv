#ifndef QUATERNION_H
#define QUATERNION_H

#include "matrix.h"
#include "vector.h"

typedef struct { float x, y, z, w; } Quaternion;

extern Quaternion const IDENTITY_QUATERNION;
extern Quaternion const ZERO_QUATERNION;

Quaternion quaternion(float, float, float, float);
void       quaternion_print(Quaternion);
float      quaternion_dot_product(Quaternion, Quaternion);
float      quaternion_norm2(Quaternion);
float      quaternion_norm(Quaternion);
Matrix     quaternion_to_matrix(Quaternion);
Quaternion quaternion_from_point(Vector);
Vector     quaternion_to_point(Quaternion);
Quaternion quaternion_from_matrix(Matrix);
Quaternion quaternion_from_matrix2(Matrix);
Quaternion quaternion_from_vector(Vector axis, float angle);
Quaternion quaternion_from_euler(float roll, float pitch, float yaw);
Quaternion quaternion_conjugate(Quaternion);
Quaternion quaternion_invert(Quaternion);
Quaternion quaternion_normalize(Quaternion);
Quaternion quaternion_scale(Quaternion, float);
Quaternion quaternion_product(Quaternion, Quaternion);
Quaternion quaternion_sandwich_product(Quaternion, Quaternion, Quaternion);
Quaternion quaternion_interpolate(Quaternion, Quaternion, float);
Quaternion quaternion_from_cube(int value);
Quaternion quaternion_root(Quaternion);
Quaternion quaternion_add(Quaternion, Quaternion);
Quaternion quaternion_sub(Quaternion, Quaternion);
int        quaternion_is_nan(Quaternion);
Vector     quaternion_rotate(Quaternion, Vector);

#endif
