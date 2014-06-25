/* Copyright Claude Knaus. All rights reserved. */

#include <assert.h>
#include <stdio.h>

#include "math_.h"
#include "matrix.h"
#include "quaternion.h"
#include "opengl.h"
#include "viewport.h"

Matrix const IDENTITY_MATRIX =
{
    {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }
};

Matrix const BIAS_MATRIX =
{
    {
        {0.5, 0, 0, 0},
        {0, 0.5, 0, 5},
        {0, 0, 0.5, 0},
        {0.5, 0.5, 0.5, 1}
    }
};

void matrix_apply(Matrix const * m)
{
    glMultMatrixf((GLfloat const *) m->m);
}

void matrix_load(Matrix const * m)
{
    glLoadMatrixf((GLfloat const *) m->m);
}

Matrix matrix_transformer(void)
{
    Viewport viewport;
    Matrix projection, model_view;

    glGetIntegerv(GL_VIEWPORT, (GLint *) &viewport);
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *) &projection);
    glGetFloatv(GL_MODELVIEW_MATRIX,  (GLfloat *) &model_view);

    return matrix_product(viewport_matrix(viewport), matrix_product(projection, model_view));
}

Matrix matrix_product(Matrix const a, Matrix const b)
{
    Matrix result;
    unsigned i, j;

    for (i = 0; i != 4; ++ i)
    {
        for (j = 0; j != 4; ++ j)
        {
            result.m[i][j] =
                b.m[i][0] * a.m[0][j] +
                b.m[i][1] * a.m[1][j] +
                b.m[i][2] * a.m[2][j] +
                b.m[i][3] * a.m[3][j];
        }
    }

    return result;
}

Vector matrix_mul(Matrix const m, Vector const v)
{
    Vector const result =
    {
        m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0],
        m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1],
        m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2]
    };
    float const w = 
        m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3];

    assert(fabs(w) >= 0.0001);

    return vector_scale(result, 1 / w);
}

Vector matrix_mul_normal(Matrix const i, Vector const n)
{
    Vector const result =
    {
        i.m[0][0] * n.x + i.m[0][1] * n.y + i.m[0][2] * n.z + i.m[0][3],
        i.m[1][0] * n.x + i.m[1][1] * n.y + i.m[1][2] * n.z + i.m[1][3],
        i.m[2][0] * n.x + i.m[2][1] * n.y + i.m[2][2] * n.z + i.m[2][3]
    };

    return result;
}

Matrix matrix_translate(Vector const v)
{
    Matrix m =
    {
        {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {v.x, v.y, v.z, 1}
        }
    };

    return m;
}

Matrix matrix_scale(Vector const v)
{
    Matrix m =
    {
        {
            {v.x, 0, 0, 0},
            {0, v.y, 0, 0},
            {0, 0, v.z, 0},
            {0, 0, 0,   1}
        }
    };

    return m;
}

void matrix_print(Matrix const m)
{
    unsigned i;

    for (i = 0; i != 4; ++ i)
    {
        printf("[%.2g, %.2g, %.2g, %.2g]\n",
            m.m[0][i], m.m[1][i], m.m[2][i], m.m[3][i]);
    }
}

static int looks_like_frustum(Matrix const m)
{
    return m.m[2][3] == -1 && m.m[3][3] == 0;
}

static int looks_like_ortho(Matrix const m)
{
    return m.m[2][3] == 0 && m.m[3][3] == 1;
}

static Projection invert_frustum(Matrix m)
{
    assert(m.m[1][0] == 0);
    assert(m.m[3][0] == 0);
    assert(m.m[0][1] == 0);
    assert(m.m[3][1] == 0);
    assert(m.m[0][2] == 0);
    assert(m.m[1][2] == 0);
    assert(m.m[0][3] == 0);
    assert(m.m[1][3] == 0);
    assert(m.m[2][3] == -1);
    assert(m.m[3][3] == 0);

    float const E = m.m[0][0];
    float const F = m.m[1][1];
    float const A = m.m[2][0];
    float const B = m.m[2][1];
    float const C = m.m[2][2];
    float const D = m.m[3][2];

    float n = D / (C - 1);

    Projection projection =
    {
        MATRIX_FRUSTUM,
        n * (A - 1) / E,
        n * (A + 1) / E,
        n * (B - 1) / F,
        n * (B + 1) / F,
        D / (C - 1),
        D / (C + 1)
    };

    return projection;
}

static Projection invert_ortho(Matrix m)
{
    assert(m.m[1][0] == 0);
    assert(m.m[2][0] == 0);
    assert(m.m[0][1] == 0);
    assert(m.m[2][1] == 0);
    assert(m.m[0][2] == 0);
    assert(m.m[1][2] == 0);
    assert(m.m[0][3] == 0);
    assert(m.m[1][3] == 0);
    assert(m.m[2][3] == 0);
    assert(m.m[3][3] == 1);

    float const A = m.m[0][0];
    float const B = m.m[1][1];
    float const C = m.m[2][2];
    float const D = m.m[3][0];
    float const E = m.m[3][1];
    float const F = m.m[3][2];

    Projection projection =
    {
        MATRIX_ORTHO,
        (D + 1) / A,
        (D + 3) / A,
        (E + 1) / B,
        (E + 3) / B,
        (F + 1) / C,
        (F - 1) / C
    };

    return projection;
}

Projection matrix_invert_projection(Matrix m)
{
    if (looks_like_frustum(m))
        return invert_frustum(m);

    if (looks_like_ortho(m))
        return invert_ortho(m);

    Projection unknown = {MATRIX_UNKNOWN, 0, 0, 0, 0, 0, 0};
    return unknown;
}

/*
 * 4x4 matrix inversion using 92 multiplications, 47 additions, 1 division.
 */
static float invert8(float const s[16], float d[16])
{
    float t[6], det, scale;
    unsigned i;

    /* first set of 2x2 determinants: 12 multiplications, 6 additions */
    t[0] = s[ 2] * s[ 7] - s[ 6] * s[ 3];
    t[1] = s[ 2] * s[11] - s[10] * s[ 3];
    t[2] = s[ 2] * s[15] - s[14] * s[ 3];
    t[3] = s[ 6] * s[11] - s[10] * s[ 7];
    t[4] = s[ 6] * s[15] - s[14] * s[ 7];
    t[5] = s[10] * s[15] - s[14] * s[11];

    /* first half of comatrix: 24 multiplications, 16 additions */
    d[ 0] = s[ 5] * t[5] - s[ 9] * t[4] + s[13] * t[3];
    d[ 1] = s[ 9] * t[2] - s[13] * t[1] - s[ 1] * t[5];
    d[ 2] = s[13] * t[0] - s[ 5] * t[2] + s[ 1] * t[4];
    d[ 3] = s[ 5] * t[1] - s[ 1] * t[3] - s[ 9] * t[0];
    d[ 4] = s[ 8] * t[4] - s[ 4] * t[5] - s[12] * t[3];
    d[ 5] = s[ 0] * t[5] - s[ 8] * t[2] + s[12] * t[1];
    d[ 6] = s[ 4] * t[2] - s[12] * t[0] - s[ 0] * t[4];
    d[ 7] = s[ 0] * t[3] - s[ 4] * t[1] + s[ 8] * t[0];

    /* second set of 2x2 determinants: 12 multiplications, 6 additions */
    t[0] = s[ 0] * s[ 5] - s[ 4] * s[ 1];
    t[1] = s[ 0] * s[ 9] - s[ 8] * s[ 1];
    t[2] = s[ 0] * s[13] - s[12] * s[ 1];
    t[3] = s[ 4] * s[ 9] - s[ 8] * s[ 5];
    t[4] = s[ 4] * s[13] - s[12] * s[ 5];
    t[5] = s[ 8] * s[13] - s[12] * s[ 9];

    /* second half of comatrix: 24 multiplications, 16 additions */
    d[ 8] = s[ 7] * t[5] - s[11] * t[4] + s[15] * t[3];
    d[ 9] = s[11] * t[2] - s[15] * t[1] - s[ 3] * t[5];
    d[10] = s[15] * t[0] - s[ 7] * t[2] + s[ 3] * t[4];
    d[11] = s[ 7] * t[1] - s[ 3] * t[3] - s[11] * t[0];
    d[12] = s[10] * t[4] - s[ 6] * t[5] - s[14] * t[3];
    d[13] = s[ 2] * t[5] - s[10] * t[2] + s[14] * t[1];
    d[14] = s[ 6] * t[2] - s[14] * t[0] - s[ 2] * t[4];
    d[15] = s[ 2] * t[3] - s[ 6] * t[1] + s[10] * t[0];

    /* determinant: 4 multiplications, 3 additions */
    det = s[0] * d[0] + s[4] * d[1] + s[8] * d[2] + s[12] * d[3];
    if (det == 0)
        return 0;
    
    /* division: 16 multiplications, 1 division */
    scale = (float) 1.0 / det;
    for (i = 0; i != 16; ++ i)
    {
        d[i] *= scale;
    }

    return det;
}

Matrix matrix_invert(Matrix const m)
{
    Matrix i;
    invert8((float const *) &m, (float *) &i);

    return i;
}

/* 4x4 matrix determinant: 24 multiplications, 14 additions */
float matrix_det(Matrix const m)
{
    float const * s = (float const *) &m;
    float d[4], t[6];

    /* first set of 2x2 determinants: 12 multiplications, 6 additions */
    t[0] = s[ 2] * s[ 7] - s[ 6] * s[ 3];
    t[1] = s[ 2] * s[11] - s[10] * s[ 3];
    t[2] = s[ 2] * s[15] - s[14] * s[ 3];
    t[3] = s[ 6] * s[11] - s[10] * s[ 7];
    t[4] = s[ 6] * s[15] - s[14] * s[ 7];
    t[5] = s[10] * s[15] - s[14] * s[11];

    /* first half of comatrix: 12 multiplications, 8 additions */
    d[ 0] = s[ 5] * t[5] - s[ 9] * t[4] + s[13] * t[3];
    d[ 1] = s[ 9] * t[2] - s[13] * t[1] - s[ 1] * t[5];
    d[ 2] = s[13] * t[0] - s[ 5] * t[2] + s[ 1] * t[4];
    d[ 3] = s[ 5] * t[1] - s[ 1] * t[3] - s[ 9] * t[0];

    /* determinant: 4 multiplications, 3 additions */
    return s[0] * d[0] + s[4] * d[1] + s[8] * d[2] + s[12] * d[3];
}

Matrix matrix_frustum(float l, float r, float b, float t, float n, float f)
{
    float
	C = (n + f) / (n - f),
	D = 2 * n * f / (n - f),
	A = (r + l) / (r - l),
	E = 2 * n / (r - l),
	B = (t + b) / (t - b),
	F = 2 * n / (t - b);
	
    Matrix projection =
    {
        {
            {E, 0, 0, 0},
            {0, F, 0, 0},
            {A, B, C,-1},
            {0, 0, D, 0}
        }
    };
	
    return projection;
}

Matrix matrix_projection(Projection p)
{
    return matrix_frustum(p.left, p.right, p.bottom, p.top, p.near_, p.far_);
}

Matrix matrix_ortho(float l, float r, float b, float t, float n, float f)
{
    float
	A = + 2 / (r - l),
	B = + 2 / (t - b),
	C = - 2 / (f - n),
	D = - (r + l) / (r - l),
	E = - (t + b) / (t - b),
	F = - (f + n) / (f - n);
	
    Matrix projection =
    {
        {
            {A, 0, 0, 0},
            {0, B, 0, 0},
            {0, 0, C, 0},
            {D, E, F, 1}
        }
    };
	
    return projection;
}

Matrix matrix_mirror(float const p[4])
{
    assert(p);

    Matrix m =
    {
        {
            {1 - 2 * p[0] * p[0],   - 2 * p[0] * p[1],   - 2 * p[0] * p[2], 0},
            {  - 2 * p[1] * p[0], 1 - 2 * p[1] * p[1],   - 2 * p[1] * p[2], 0},
            {  - 2 * p[2] * p[0],   - 2 * p[2] * p[1], 1 - 2 * p[2] * p[2], 0},
            {  - 2 * p[3] * p[0],   - 2 * p[3] * p[1],   - 2 * p[3] * p[2], 1}
        }
    };

    return m;
}

Matrix matrix_shadow(float const p[4], float const l[4])
{
    float d = p[0] * l[0] + p[1] * l[1] + p[2] * l[2] + p[3] * l[3];
    Matrix m =
    {
        {
            {d - l[0] * p[0],   - l[1] * p[0],   - l[2] * p[0],   - l[3] * p[0]},
            {  - l[0] * p[1], d - l[1] * p[1],   - l[2] * p[1],   - l[3] * p[1]},
            {  - l[0] * p[2],   - l[1] * p[2], d - l[2] * p[2],   - l[3] * p[2]},
            {  - l[0] * p[3],   - l[1] * p[3],   - l[2] * p[3], d - l[3] * p[3]}
        }
    };

    return m;
}

Matrix matrix_rotate(Vector const axis, float const angle)
{
    float const length = sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    float const angle_2 = angle / 2;
    float const c = cos(angle_2);
    float const sl = sin(angle_2) / length;
    Quaternion const q = {axis.x * sl, axis.y * sl, axis.z * sl, c};

    return quaternion_to_matrix(q);
}

float matrix_axis(Matrix const m, Vector * const axis)
{
    Quaternion const q = quaternion_from_matrix(m);
    float const c = q.w;
    float const angle = acos(c) * 2;
    float s = sqrt(1 - c * c);

    if (fabs(s) < 0.0005)
        s = 1;

    axis->x = q.x / s;
    axis->y = q.y / s;
    axis->z = q.z / s;

    return angle;
}

Matrix matrix_look_at(Vector const position, Vector const object, Vector up)
{
    Vector const direction = vector_normalize(vector_sub(object, position));
    up = vector_normalize(up);
   
    Vector const side = cross(direction, up);
    up = cross(side, direction);

    Matrix const m =
    {
        {
            {side.x, side.y, side.z, 0},
            {up.x, up.y, up.z, 0},
            {-direction.x, -direction.y, -direction.z, 0},
            {0, 0, 0, 1}
        }
    }; 

    return m;
}

Matrix matrix_transpose(Matrix const matrix)
{
    Matrix m;
    int i, j;

    for (i = 0; i != 4; ++ i)
    for (j = 0; j != 4; ++ j)
    {
        m.m[i][j] = matrix.m[j][i];
    }

    return m;
}

Matrix matrix_scale2(Matrix const matrix, float f)
{
    Matrix m;
    int i, j;

    for (i = 0; i != 4; ++ i)
    for (j = 0; j != 4; ++ j)
    {
        m.m[i][j] = matrix.m[i][j] * f;
    }

    return m;
}

Matrix matrix_diagonal(float a, float b, float c, float d)
{
    Matrix m =
    {
        {
            {a, 0, 0, 0},
            {0, b, 0, 0},
            {0, 0, c, 0},
            {0, 0, 0, d}
        }
    };

    return m;
}

Matrix matrix_from_bases(Vector x, Vector y, Vector z)
{
    Matrix m =
    {
        {
            {x.x, x.y, x.z, 0},
            {y.x, y.y, y.z, 0},
            {z.x, z.y, z.z, 0},
            {  0,   0,   0, 1}
        }
    };

    return m;
}

Matrix matrix_interpolate(Matrix m1, Matrix m2, float f)
{
    Matrix m;
    int i, j;

    for (i = 0; i != 4; ++ i)
    for (j = 0; j != 4; ++ j)
    {
        m.m[i][j] = interpolate(m1.m[i][j], m2.m[i][j], f);
    }

    return m;
}

Vector vector_rotate(Vector v, Vector axis, float angle)
{
    Matrix m = matrix_rotate(axis, angle);
    return matrix_mul(m, v);
}

static char const * projection_type_to_string(Projection_Type type)
{
    switch (type)
    {
        default:             return "unknown";
        case MATRIX_ORTHO:   return "orthogonal";
        case MATRIX_FRUSTUM: return "perspective";
    } 
}

void projection_print(Projection projection)
{
    printf("projection type = %s\n", projection_type_to_string(projection.type));
    printf("(left, right) = (%g, %g)\n", projection.left, projection.right);
    printf("(bottom, top) = (%g, %g)\n", projection.bottom, projection.top);
    printf("(near, far) = (%g, %g)\n", projection.near_, projection.far_);
}

Matrix matrix_tensor(Vector a, Vector b)
{
    Matrix m =
    {
        {
            {a.x * b.x, a.y * b.x, a.z * b.x, 0},
            {a.x * b.y, a.y * b.y, a.z * b.y, 0},
            {a.x * b.z, a.y * b.z, a.z * b.z, 0},
            {        0,         0,         0, 1}
        }
    };

    return m;
}
