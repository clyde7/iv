#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

typedef struct {float m[4][4];} Matrix;

typedef enum   {MATRIX_UNKNOWN, MATRIX_FRUSTUM, MATRIX_ORTHO} Projection_Type;
typedef struct {Projection_Type type; float left, right, bottom, top, near_, far_;} Projection;

extern Matrix const IDENTITY_MATRIX;
extern Matrix const BIAS_MATRIX;

Matrix matrix_transformer(void);
void   matrix_apply(Matrix const *);
void   matrix_load(Matrix const *);
void   matrix_print(Matrix);
Vector matrix_mul(Matrix, Vector);
Vector matrix_mul_normal(Matrix, Vector const n);
Matrix matrix_product(Matrix, Matrix);
Matrix matrix_tensor(Vector, Vector);
Matrix matrix_translate(Vector);
Matrix matrix_scale(Vector);
Matrix matrix_rotate(Vector, float);
Matrix matrix_look_at(Vector position, Vector direction, Vector up);
Matrix matrix_projection(Projection);
Matrix matrix_frustum(float l, float r, float b, float t, float n, float f);
Matrix matrix_ortho(float l, float r, float b, float t, float n, float f);
Matrix matrix_mirror(float const plane[4]); /* TODO use normal and point*/
Matrix matrix_shadow(float const plane[4], float const light[4]); /* use normal, point, ...? */
Matrix matrix_invert(Matrix);
float  matrix_det(Matrix);
float  matrix_axis(Matrix, Vector *);
Matrix matrix_transpose(Matrix);
Matrix matrix_diagonal(float, float, float, float);
Matrix matrix_scale2(Matrix, float);
Matrix matrix_from_bases(Vector x, Vector y, Vector z);
Matrix matrix_interpolate(Matrix m1, Matrix m2, float f);
void   matrix_test(void);

Projection matrix_invert_projection(Matrix);
void projection_print(Projection);

#endif
