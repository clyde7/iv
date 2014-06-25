/* Copyright Claude Knaus. All rights reserved. */

#include <assert.h>
#include <stdio.h>

#include "math_.h"
#include "quaternion.h"

Quaternion const IDENTITY_QUATERNION = {0, 0, 0, 1};
Quaternion const ZERO_QUATERNION = {0, 0, 0, 0};

Quaternion quaternion(float x, float y, float z, float w)
{
    Quaternion q = {x, y, z, w};
    return q;
}

Quaternion quaternion_from_point(Vector v)
{
    Quaternion q = {v.x, v.y, v.z, 0};
    return q;
}

Vector quaternion_to_point(Quaternion q)
{
    Vector v = {q.x, q.y, q.z};
    return v;
}

void quaternion_print(Quaternion q)
{
    printf("{%f, %f, %f}, %f", q.x, q.y, q.z, q.w);
}

Matrix quaternion_to_matrix(Quaternion const q)
{
    float const
        xx = q.x * q.x,
        xy = q.x * q.y,
        xz = q.x * q.z,
        xw = q.x * q.w,
        yy = q.y * q.y,
        yz = q.y * q.z,
        yw = q.y * q.w,
        zz = q.z * q.z,
        zw = q.z * q.w;

    Matrix const m =
    {
        {
            {1 - (yy + zz) * 2,     (xy + zw) * 2,     (xz - yw) * 2, 0},
            {    (xy - zw) * 2, 1 - (xx + zz) * 2,     (yz + xw) * 2, 0},
            {    (xz + yw) * 2,     (yz - xw) * 2, 1 - (xx + yy) * 2, 0},
            {                0,                 0,                 0, 1}
        }
    };

    return m;
}

Quaternion quaternion_from_matrix(Matrix const m)
{
    float const trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
	
    if (trace > 0)
    {
        float const s = sqrt(trace + 1);
        float const t = 0.5 / s;
        Quaternion const q =
        {
            (m.m[1][2] - m.m[2][1]) * t,
            (m.m[2][0] - m.m[0][2]) * t,
            (m.m[0][1] - m.m[1][0]) * t,
            s / 2
        };
		
        return q;
    }
	
    int min_index = 0;
    if (m.m[1][1] > m.m[0][0])
        min_index = 1;
    if (m.m[2][2] > m.m[min_index][min_index])
        min_index = 2;
	
    /*printf("min_index = %d\n", min_index);*/
	
    switch (min_index)
    {
        default:
            assert(0);
			
        case 0:
        {
            float const s = sqrt(m.m[0][0] - m.m[1][1] - m.m[2][2] + 1);
            float const t = (s == 0.0) ? s : (0.5 / s);
            Quaternion const q =
            {
                s / 2,
                (m.m[0][1] + m.m[1][0]) * t,
                (m.m[0][2] + m.m[2][0]) * t,
                (m.m[1][2] - m.m[2][1]) * t
            };
            return q;
        }
			
        case 1:
        {
            float const s = sqrt(- m.m[0][0] + m.m[1][1] - m.m[2][2] + 1);
            float const t = (s == 0.0) ? s : (0.5 / s);
            Quaternion const q =
            {
                (m.m[1][0] + m.m[0][1]) * t,
                s / 2,
                (m.m[1][2] + m.m[2][1]) * t,
                (m.m[2][0] - m.m[0][2]) * t
            };
            return q;
        }
			
        case 2:
        {
            float const s = sqrt(- m.m[0][0] - m.m[1][1] + m.m[2][2] + 1);
            float const t = (s == 0.0) ? s : (0.5 / s);
            Quaternion const q =
            {
                (m.m[2][0] + m.m[0][2]) * t,
                (m.m[2][1] + m.m[1][2]) * t,
                s / 2,
                (m.m[0][1] - m.m[1][0]) * t
            };
            return q;
        }
    }
}

Quaternion quaternion_from_matrix2(Matrix m)
{
    Quaternion q;
    float  tr, s;
    int    i, j, k;
	
	int nxt[3] = {1, 2, 0};
	
	tr = m.m[0][0] + m.m[1][1] + m.m[2][2];
	
	if (tr > 0.0) {
		s = sqrt(tr + 1.0);
		q.w = s / 2.0;
		s = 0.5 / s;
		q.x = (m.m[1][2] - m.m[2][1]) * s;
		q.y = (m.m[2][0] - m.m[0][2]) * s;
		q.z = (m.m[0][1] - m.m[1][0]) * s;
	} else {                
		float qa[4];
		/* diagonal is negative */
		i = 0;
		if (m.m[1][1] > m.m[0][0]) i = 1;
		if (m.m[2][2] > m.m[i][i]) i = 2;
		j = nxt[i];
		k = nxt[j];
		
		s = sqrt ((m.m[i][i] - (m.m[j][j] + m.m[k][k])) + 1.0);
		qa[i] = s * 0.5;
		
		if (s != 0.0) s = 0.5 / s;
		
		qa[3] = (m.m[j][k] - m.m[k][j]) * s;
		qa[j] = (m.m[i][j] + m.m[j][i]) * s;
		qa[k] = (m.m[i][k] + m.m[k][i]) * s;
		
		q.x = qa[0];
		q.y = qa[1];
		q.z = qa[2];
		q.w = qa[3];
	}
	
    return q;
}

Quaternion quaternion_from_vector(Vector v, float angle)
{
    // TODO test for v.length = 0
    v = vector_normalize(v);

    float c = cos(angle / 2);
    float s = sin(angle / 2);

    Quaternion q = { v.x * s, v.y * s, v.z * s, c };

    return q;
}

float quaternion_norm2(Quaternion q)
{
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

float quaternion_norm(Quaternion q)
{
    return sqrt(quaternion_norm2(q));
}

Quaternion quaternion_conjugate(Quaternion q)
{
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;

    return q;
}

Quaternion quaternion_invert(Quaternion q)
{
    Quaternion q_ = quaternion_conjugate(q);
    float norm2 = quaternion_norm2(q);

    return quaternion_scale(q_, norm2);
}

Quaternion quaternion_scale(Quaternion q, float factor)
{
    q.x *= factor;
    q.y *= factor;
    q.z *= factor;
    q.w *= factor;
    
    return q;
}

Quaternion quaternion_normalize(Quaternion q)
{
    float factor = 1.0 / quaternion_norm(q);
    return quaternion_scale(q, factor);
}

Quaternion quaternion_add(Quaternion p, Quaternion q)
{
    Quaternion r =
    {
        p.x + q.x,
        p.y + q.y,
        p.z + q.z,
        p.w + q.w
    };

    return r;
}

Quaternion quaternion_sub(Quaternion p, Quaternion q)
{
    Quaternion r =
    {
        p.x - q.x,
        p.y - q.y,
        p.z - q.z,
        p.w - q.w
    };

    return r;
}

Quaternion quaternion_product(Quaternion p, Quaternion q)
{
    Quaternion r =
    {
        p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
        p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z,
        p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x,
        p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z
    };

    return r;
}

float quaternion_dot_product(Quaternion p, Quaternion q)
{
    return 
        p.x * q.x +
        p.y * q.y +
        p.z * q.z +
        p.w * q.w;
}

Quaternion quaternion_from_euler(float roll, float pitch, float yaw)
{
    float cr = cos(roll / 2);
    float cp = cos(pitch / 2);
    float cy = cos(yaw / 2);

    float sr = sin(roll / 2);
    float sp = sin(pitch / 2);
    float sy = sin(yaw / 2);
        
    Quaternion q =
    {
        sr * cp * cy - cr * sp * sy,
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy,
        cr * cp * cy + sr * sp * sy
    };

    return q;
}

Quaternion quaternion_interpolate(Quaternion p, Quaternion q, float t)
{
    float scale0, scale1;
    float c_omega = p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w;

    if (c_omega < 0.0)
    {
        c_omega = -c_omega;
        q.x = - q.x;
        q.y = - q.y;
        q.z = - q.z;
        q.w = - q.w;
    }

    if (1.0 - c_omega > 0.01)
    {
        float omega = acos(c_omega);
        float s_omega = sin(omega);
        scale0 = sin((1 - t) * omega) / s_omega;
        scale1 = sin(t * omega) / s_omega;
    }
    else
    {        
        scale0 = 1 - t;
        scale1 = t;
    }

    Quaternion r =
    {
        scale0 * p.x + scale1 * q.x,
        scale0 * p.y + scale1 * q.y,
        scale0 * p.z + scale1 * q.z,
        scale0 * p.w + scale1 * q.w
    };

    return r;
}

Quaternion quaternion_from_cube_corner(int corner)
{
    float d = sqrt(2.0) / 2;

    switch (corner)
    {
        default: assert(0);
        case 0: return quaternion(0, 0, 0, 1);
        case 1: return quaternion(0, d, 0, d);
        case 2: return quaternion(0, 0, d, d);
        case 3: return quaternion(0, 0, 1, 0);
        case 4: return quaternion(d, 0, 0, d);
        case 5: return quaternion(0, 1, 0, 0);
        case 6: return quaternion(1, 0, 0, 0);
        case 7: return quaternion(0, 0, 0,-1);
    }
}

Quaternion quaternion_from_cube_dimension(int dimension)
{
    switch (dimension)
    {
        default: assert(0);
        case 0: return quaternion(0, 0, 0, 1);
        case 1: return quaternion(0.5, 0.5, 0.5, 0.5);
        case 2: return quaternion(0.5, 0.5, 0.5,-0.5);
    }
}

Quaternion quaternion_from_cube(int value)
{
    assert(value >= 0 && value < 24);

    Quaternion dimension_q = quaternion_from_cube_dimension(value / 8);
    Quaternion corner_q = quaternion_from_cube_corner(value % 8);

    return quaternion_product(dimension_q, corner_q);
}

Quaternion quaternion_root(Quaternion q)
{
    // XXX alternative:
#if 0
    if (q.w < -1 + 0.00001)
        return IDENTITY_QUATERNION;
#endif
    // XXX or any direction orthogonal, i.e., (vector, 0)

    // XXX what is the root?
    if (q.w < -1 + 0.00001)
        q.w = -q.w;

//    if (fabs(q.w + 1) < 0.000001)
 //       return q;

    q.w += 1;
    float factor = 1.0 / sqrt(2 * q.w);
    return quaternion_scale(q, factor);
}

Quaternion quaternion_sandwich_product(Quaternion a, Quaternion b, Quaternion c)
{
    return quaternion_product(a, quaternion_product(b, c));
}

int quaternion_is_nan(Quaternion q)
{
#ifdef WINDOWS
	return 0; // XXX
#else
    return
        isnan(q.x) ||
        isnan(q.y) ||
        isnan(q.z) ||
        isnan(q.w);
#endif
}

Vector quaternion_rotate(Quaternion q, Vector v)
{
    Quaternion p = quaternion_from_point(v);
    p = quaternion_sandwich_product(q, p, quaternion_conjugate(q));
    return quaternion_to_point(p);
}
