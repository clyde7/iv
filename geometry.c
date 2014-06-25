#include <stdlib.h>
#include <stdio.h>

#include "box.h"
#include "color.h"
#include "error.h"
#include "font.h"
#include "math_.h"
#include "memory.h"
#include "opengl.h"
#include "geometry.h"

Vector const tetrahedron_vertices[4] =
{
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
};

Vector const cube_vertices[8] =
{
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {0, 1, 1},
    {1, 1, 1}
};

Vector const unit_cube_vertices[8] =
{
    {-1, -1, -1},
    {+1, -1, -1},
    {-1, +1, -1},
    {+1, +1, -1},
    {-1, -1, +1},
    {+1, -1, +1},
    {-1, +1, +1},
    {+1, +1, +1}
};

Vector const octahedron_vertices[6] =
{
    {-1,  0,  0},
    {+1,  0,  0},
    { 0, -1,  0},
    { 0, +1,  0},
    { 0,  0, -1},
    { 0,  0, +1}
};

Vector const cuboctahedron_vertices[12] =
{
    { 0, -1, -1},
    { 0, +1, -1},
    { 0, -1, +1},
    { 0, +1, +1},
    {-1,  0, -1},
    {+1,  0, -1},
    {-1,  0, +1},
    {+1,  0, +1},
    {-1, -1,  0},
    {+1, -1,  0},
    {-1, +1,  0},
    {+1, +1,  0}
};

static Vector const sky_box_vertices[8] =
{
    {-1, -1, -1},
    {+1, -1, -1},
    {-1, +1, -1},
    {+1, +1, -1},
    {-1, -1, +1},
    {+1, -1, +1},
    {-1, +1, +1},
    {+1, +1, +1}
};

#if 0

typedef struct { unsigned a, b, c; } Face;

Array create_tetrahedron2(void)
{
    float tau = asin(1.0 / 3);
    float r = cos(tau);
    float z = sin(tau);

    float phi = 2 * M_PI / 3;
    float c = cos(phi);
    float s = sin(phi);

    Vector * vertices = malloc(4 * sizeof(Vector));
    vertices[0] = vector(0, 0, 1);
    vertices[1] = vector(r, 0, -z);
    vertices[2] = vector(r * c, + r * s, -z);
    vertices[3] = vector(r * c, - r * s, -z);

    Face * faces = malloc(4 * sizeof(Face));
    faces[0] = face(0, 1, 2);
    faces[1] = face(0, 2, 3);
    faces[2] = face(0, 3, 1);
    faces[3] = face(3, 2, 1);

    Array array = 
    {
        {1, 1, 1};
        vertices,
        NULL,
        NULL,
        4,
        4 * 3,
        faces
    };

    return array(vertex_count, vertices, NULL, NULL, face_count, faces, IDENTITY_MATRIX);
}
#endif

void create_tetrahedron(Vector vertices[4])
{
    float tau = asin(1.0 / 3);
    float r = cos(tau);
    float z = sin(tau);

    float phi = 2 * M_PI / 3;
    float c = cos(phi);
    float s = sin(phi);

    Vector p0 = {0, 0, 1};
    Vector p1 = {r, 0, -z};
    Vector p2 = {r * c, + r * s, -z};
    Vector p3 = {r * c, - r * s, -z};

    vertices[0] = p0;
    vertices[1] = p1;
    vertices[2] = p2;
    vertices[3] = p3;
}

static void circle(GLenum mode, Vector position, float radius, int count)
{
    glBegin(mode);

    for (int i = 0; i != count; ++ i)
    {
        float t = (float) i / count;
        float phi = t * M_PI * 2;

        Vector v = vector_polar(phi, radius);
        v = vector_add(v, position);

        vector_apply(v);
    }

    glEnd();
}

static void ellipse(GLenum mode, Vector position, float a, float b, int count)
{
    glBegin(mode);

    for (int i = 0; i != count; ++ i)
    {
        float t = (float) i / count;
        float phi = t * M_PI * 2;

        float p = b * cos(t);
        float q = a * sin(t);
        float radius = a * b / sqrt(p*p + q*q);

        Vector v = vector_polar(phi, radius);
        v = vector_add(v, position);

        vector_apply(v);
    }

    glEnd();
}

void draw_circle(Vector position, float radius, int count)
{
    circle(GL_LINE_LOOP, position, radius, count);
}

void fill_circle(Vector position, float radius, int count)
{
    circle(GL_POLYGON, position, radius, count);
}

void draw_ellipse(Vector position, float a, float b, int count)
{
    ellipse(GL_LINE_LOOP, position, a, b, count);
}

void fill_ellipse(Vector position, float a, float b, int count)
{
    ellipse(GL_POLYGON, position, a, b, count);
}

void draw_sphere(Vector position, float radius, int nx, int ny)
{
    float r, z, theta;
    int i, j;

    /* north cap */
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex3f(position.x, position.y, position.z + radius);

    theta = (float) M_PI * (ny - 1) / ny - M_PI/2;
    r = radius * cos(theta);
    z = radius * sin(theta);

    glNormal3f(cos(theta), 0, sin(theta));
    glVertex3f(position.x + r, position.y, position.z + z);

    for (i = 1; i != nx; ++ i)
    {
        float phi = (float) 2 * M_PI * i / nx;
        float x = r * cos(phi);
        float y = r * sin(phi);
        Vector normal = {x / radius, y / radius, z / radius};

        glNormal3fv((GLfloat const *) &normal);
        glVertex3f(position.x + x, position.y + y, position.z + z);
    }

    glNormal3f(cos(theta), 0, sin(theta));
    glVertex3f(position.x + r, position.y, position.z + z);

    glEnd();

    /* south cap */
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    glVertex3f(position.x, position.y, position.z - radius);

    glNormal3f(cos(theta), 0, -sin(theta));
    glVertex3f(position.x + r, position.y, position.z - z);

    for (i = 1; i != nx; ++ i)
    {
        float phi = /*2 * M_PI -*/ (float) 2 * M_PI * i / nx;
        float x = r * cos(phi);
        float y = r * sin(phi);
        Vector normal = {x / radius, y / radius, -z / radius};

        glNormal3fv((GLfloat const *) &normal);
        glVertex3f(position.x + x, position.y + y, position.z - z);
    }

    glNormal3f(cos(theta), 0, -sin(theta));
    glVertex3f(position.x + r, position.y, position.z - z);

    glEnd();

    /* body */
    for (i = 1; i != ny - 1; ++ i)
    {
        float theta1 = (float) M_PI * (i + 0) / ny - M_PI/2;
        float r1 = radius * cos(theta1);
        float z1 = radius * sin(theta1);

        float theta2 = (float) M_PI * (i + 1) / ny - M_PI/2;
        float r2 = radius * cos(theta2);
        float z2 = radius * sin(theta2);

        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j <= nx; ++ j)
        {
            float phi = (float) 2 * M_PI * j / nx;
            float c = cos(phi);
            float s = sin(phi);

            float x1 = r1 * c;
            float y1 = r1 * s;
            Vector normal1 = {x1 / radius, y1 / radius, z1 / radius};

            float x2 = r2 * c;
            float y2 = r2 * s;
            Vector normal2 = {x2 / radius, y2 / radius, z2 / radius};

            glNormal3fv((GLfloat const *) &normal2);
            glVertex3f(position.x + x2, position.y + y2, position.z + z2);

            glNormal3fv((GLfloat const *) &normal1);
            glVertex3f(position.x + x1, position.y + y1, position.z + z1);
        }
        glEnd();
    }
}

void draw_quad(Vector const quad[4])
{
    glBegin(GL_QUADS);
    vector_apply(quad[0]);
    vector_apply(quad[1]);
    vector_apply(quad[2]);
    vector_apply(quad[3]);
    glEnd();
}

void draw_rect(Box box)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(box.min.x, box.min.y);
    glVertex2f(box.max.x, box.min.y);
    glVertex2f(box.max.x, box.max.y);
    glVertex2f(box.min.x, box.max.y);
    glEnd();
}

void fill_rect(Box box)
{
    glBegin(GL_QUADS);
    glVertex2f(box.min.x, box.min.y);
    glVertex2f(box.max.x, box.min.y);
    glVertex2f(box.max.x, box.max.y);
    glVertex2f(box.min.x, box.max.y);
    glEnd();
}

static void draw_arc(Vector position, float radius, float angle_1, float angle_2, int n)
{
    for (int i = 0; i <= n; ++ i)
    {
        float t = (float) i / n;
        float angle = interpolate(angle_1, angle_2, t);

        Vector v = vector_polar(angle, radius);
        v = vector_add(v, position);
        vector_apply(v);
    }
}

static void rect_rounded(GLenum mode, Box box, float radius, int n)
{
    Vector size = box_size(box);
    float min_size = vector_min_component(size);
    radius = fmin(radius, min_size / 2);

    box = box_expand(box, -radius);

    Vector position = ORIGIN;

    glBegin(mode);

    position.x = box.min.x;
    position.y = box.min.y;
    draw_arc(position, radius, M_PI, M_PI * 1.5, n);

    position.x = box.max.x;
    position.y = box.min.y;
    draw_arc(position, radius, M_PI * 1.5, M_PI * 2, n);

    position.x = box.max.x;
    position.y = box.max.y;
    draw_arc(position, radius, 0, M_PI * 0.5, n);

    position.x = box.min.x;
    position.y = box.max.y;
    draw_arc(position, radius, M_PI * 0.5, M_PI, n);

    glEnd();
}

void draw_rect_rounded(Box box, float radius, int n)
{
    rect_rounded(GL_LINE_LOOP, box, radius, n);
}

void fill_rect_rounded(Box box, float radius, int n)
{
    rect_rounded(GL_POLYGON, box, radius, n);
}

Array * torus_array(Vector position, float r1, float r2, int nx, int ny)
{
    int vertex_count = nx * ny;
    int face_count = vertex_count;

    Box box = MIN_BOX;
    Vector * vertices = malloc_array(Vector, vertex_count);
    Vector * normals  = malloc_array(Vector, vertex_count);
    Color * colors    = malloc_array(Color, vertex_count);
    Quad * faces      = malloc_array(Quad, face_count);

    int i, j;

    for (i = 0; i < ny; ++ i)
    {
        float theta = 2 * M_PI * i / ny;
        float c_theta = cos(theta);
        float s_theta = sin(theta);

        for (j = 0; j < nx; ++ j)
        {
            float phi =  2 * M_PI * j / nx;
            float c_phi = cos(phi);
            float s_phi = sin(phi);

            Vector vertex =
            {
                (r1 + r2 * c_theta) * c_phi,
                (r1 + r2 * c_theta) * s_phi,
                r2 * s_theta
            };
            Vector normal = {c_theta * c_phi, c_theta * s_phi, s_theta};
            //Color color = color_wheel_hsv(phi);
            Color color = color_wheel_orgb(phi);

            box = box_add(box, vertex);

            vertices[i * nx + j] = vertex;
            normals[i * nx + j] = normal;
            colors[i * nx + j] = color;
            
            Quad * quad = &faces[i * nx + j];
            quad->a = i * nx + j;
            quad->b = i * nx + ((j + 1 == nx) ? 0 : j + 1);
            quad->c = ((i + 1 == ny) ? 0 : i + 1) * nx + ((j + 1 == nx) ? 0 : j + 1);
            quad->d = ((i + 1 == ny) ? 0 : i + 1) * nx + j;
        }
    }

    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count = face_count;
    array->vertices = vertices;
    array->normals = normals;
    array->colors = colors;
    array->faces.quads = faces;

    return array;
}

void draw_torus(float r1, float r2, int nx, int ny)
{
    for (int i = 0; i < ny; ++ i)
    {
        float theta = 2 * M_PI * i / ny;
        float c_theta = cos(theta);
        float s_theta = sin(theta);

        float theta1 = 2 * M_PI * (i + 1) / ny;
        float c_theta1 = cos(theta1);
        float s_theta1 = sin(theta1);

        glBegin(GL_TRIANGLE_STRIP);

        for (int j = 0; j <= nx; ++ j)
        {
            float phi =  2 * M_PI * j / nx;
            float c_phi = cos(phi);
            float s_phi = sin(phi);

            Vector vertex =
            {
                (r1 + r2 * c_theta) * c_phi,
                (r1 + r2 * c_theta) * s_phi,
                r2 * s_theta
            },
            vertex1 = 
            {
                (r1 + r2 * c_theta1) * c_phi,
                (r1 + r2 * c_theta1) * s_phi,
                r2 * s_theta1
            };

            Vector normal  = {c_theta * c_phi, c_theta * s_phi, s_theta};
            Vector normal1 = {c_theta1 * c_phi, c_theta1 * s_phi, s_theta1};

            glNormal3fv(&normal.x);
            glVertex3fv(&vertex.x);

            glNormal3fv(&normal1.x);
            glVertex3fv(&vertex1.x);
        }

        glEnd();
    }
}

static void moebius_vertices(float radius, float width, float phi, Vector pair[2], Vector * normal)
{
    float cos_phi = cos(phi);
    float sin_phi = sin(phi);

    float tau = phi / 2;
    float cos_tau = cos(tau);
    float sin_tau = sin(tau);

    Vector v = {radius * cos_phi, radius * sin_phi, 0};
    Vector d = {(width / 2) * cos_phi * cos_tau, (width / 2) * sin_phi * cos_tau, (width / 2) * sin_tau};
    Vector n = {- cos_phi * sin_tau, - sin_phi * sin_tau, cos_tau};

    pair[0] = vector_add(v, d);
    pair[1] = vector_sub(v, d);
    *normal = n;
}

void draw_moebius_strip(float radius, float width, int resolution)
{
    Color color;
    Vector v[2], n;
    int i;

    glBegin(GL_QUAD_STRIP);

    moebius_vertices(radius, width, 0, v, &n);
    color = color_from_hsv(0, 1, 1);
    glColor3fv((GLfloat const *) &color);
    glNormal3fv((GLfloat const *) &n);
    glVertex3fv((GLfloat const *) &v[0]);
    glVertex3fv((GLfloat const *) &v[1]);

    for (i = 1; i != resolution; ++ i)
    {
        float phi = 2 * M_PI * i / resolution;

        moebius_vertices(radius, width, phi, v, &n);
        color = color_from_hsv(phi * 180 / M_PI, 1, 1);
        color = color_from_hsv(discretize(phi * 180 / M_PI, 360, 12), 1, 1);
        glColor3fv((GLfloat const *) &color);
        glNormal3fv((GLfloat const *) &n);
        glVertex3fv((GLfloat const *) &v[0]);
        glVertex3fv((GLfloat const *) &v[1]);
    }

    moebius_vertices(radius, width, 0, v, &n);
    color = color_from_hsv(0, 1, 1);
    glColor3fv((GLfloat const *) &color);
    glNormal3fv((GLfloat const *) &n);
    glVertex3fv((GLfloat const *) &v[1]); /* swap vertices! */
    glVertex3fv((GLfloat const *) &v[0]);

    glEnd();
}

void draw_moebius_contour(float radius, float width, int resolution)
{
    Vector v[2], n;
    Color color;
    int i;

    glBegin(GL_LINE_LOOP);

    moebius_vertices(radius, width, 0, v, &n);
    color = color_from_hsv(0, 1, 1);
    glColor3fv((GLfloat const *) &color);
    glVertex3fv((GLfloat const *) &v[0]);

    for (i = 1; i != resolution * 2; ++ i)
    {
        float phi = 2 * M_PI * i / resolution;

        moebius_vertices(radius, width, phi, v, &n);
        color = color_from_hsv(phi * 180 / M_PI, 1, 1);
        glColor3fv((GLfloat const *) &color);
        glVertex3fv((GLfloat const *) &v[0]);
    }

    glEnd();
}

/* should rename c to position */
void draw_grid(Vector v1, Vector v2, Vector c, Vector d)
{
    static const float epsilon = 0.00001;
    float x, y, z;

    glBegin(GL_LINES);

    for (x = v1.x; x < v2.x + epsilon; x += d.x)
    {
        glVertex3f(x, v1.y, c.z);
        glVertex3f(x, v2.y, c.z);

        glVertex3f(x, c.y, v1.z);
        glVertex3f(x, c.y, v2.z);
    }

    for (y = v1.y; y < v2.y + epsilon; y += d.y)
    {
        glVertex3f(v1.x, y, c.z);
        glVertex3f(v2.x, y, c.z);

        glVertex3f(c.x, y, v1.z);
        glVertex3f(c.x, y, v2.z);
    }

    for (z = v1.z; z < v2.z + epsilon; z += d.z)
    {
        glVertex3f(v1.x, c.y, z);
        glVertex3f(v2.x, c.y, z);

        glVertex3f(c.x, v1.y, z);
        glVertex3f(c.x, v2.y, z);
    }

    glEnd();
}

void draw_polar_grid(Vector position, float radius, int n_rotational, int n_radial, int resolution)
{
    int i, j;

    glBegin(GL_LINES);

    for (i = 0; i != n_rotational; ++ i)
    {
        float phi = 2 * M_PI * i / n_rotational;
        float c = cos(phi);
        float s = sin(phi);

        glVertex3fv((GLfloat const *) &position);
        glVertex3f(position.x + radius * c, position.y + radius * s, position.z);
    }

    glEnd();

    for (i = 0; i <= n_radial; ++ i)
    {
        float r = radius * i / n_radial;
        glBegin(GL_LINE_LOOP);

        for (j = 0; j != resolution; ++ j)
        {
            float phi = 2 * M_PI * j / resolution;
            float c = cos(phi);
            float s = sin(phi);

            glVertex3f(position.x + r * c, position.y + r * s, position.z);
        }

        glEnd();

    }
}

void draw_rose(int n, int d, float delta)
{
    int i;
    float k = (float) n / d;
    float period = 2 * M_PI * n * d;

    glBegin(GL_LINE_STRIP);

    for (i = 0; ; ++ i)
    {
        float phi = i * delta;
        if (k * phi > period)
            break;

        float r = sin(k * phi);
        Vector v = {r * cos(phi), r * sin(phi), 0};

        vector_apply(v);
    }

    glVertex3f(0, 0, 0);

    glEnd();
}

void draw_sky_box(Box box)
{
    Vector a = box.min;
    Vector b = box.max;
    Vector vertices[8] =
    {
        {a.x, a.y, a.z},
        {b.x, a.y, a.z},
        {a.x, b.y, a.z},
        {b.x, b.y, a.z},
        {a.x, a.y, b.z},
        {b.x, a.y, b.z},
        {a.x, b.y, b.z},
        {b.x, b.y, b.z}
    };
    Vector normals[8];
    int i;

    for (i = 0; i != 8; ++ i)
    {
        normals[i] = vector_normalize(vertices[i]);
    }

    glBegin(GL_TRIANGLE_FAN);
    glNormal3fv((GLfloat *) &normals[0]); glVertex3fv((GLfloat *) &vertices[0]);
    glNormal3fv((GLfloat *) &normals[1]); glVertex3fv((GLfloat *) &vertices[1]);
    glNormal3fv((GLfloat *) &normals[3]); glVertex3fv((GLfloat *) &vertices[3]);
    glNormal3fv((GLfloat *) &normals[2]); glVertex3fv((GLfloat *) &vertices[2]);
    glNormal3fv((GLfloat *) &normals[6]); glVertex3fv((GLfloat *) &vertices[6]);
    glNormal3fv((GLfloat *) &normals[4]); glVertex3fv((GLfloat *) &vertices[4]);
    glNormal3fv((GLfloat *) &normals[5]); glVertex3fv((GLfloat *) &vertices[5]);
    glNormal3fv((GLfloat *) &normals[1]); glVertex3fv((GLfloat *) &vertices[1]);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3fv((GLfloat *) &normals[7]); glVertex3fv((GLfloat *) &vertices[7]);
    glNormal3fv((GLfloat *) &normals[1]); glVertex3fv((GLfloat *) &vertices[1]);
    glNormal3fv((GLfloat *) &normals[5]); glVertex3fv((GLfloat *) &vertices[5]);
    glNormal3fv((GLfloat *) &normals[4]); glVertex3fv((GLfloat *) &vertices[4]);
    glNormal3fv((GLfloat *) &normals[6]); glVertex3fv((GLfloat *) &vertices[6]);
    glNormal3fv((GLfloat *) &normals[2]); glVertex3fv((GLfloat *) &vertices[2]);
    glNormal3fv((GLfloat *) &normals[3]); glVertex3fv((GLfloat *) &vertices[3]);
    glNormal3fv((GLfloat *) &normals[1]); glVertex3fv((GLfloat *) &vertices[1]);
    glEnd();
}

void draw_box(Box box)
{
    Vector v1 = box.min;
    Vector v2 = box.max;

    glBegin(GL_LINE_LOOP);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v1.z);
    glVertex3f(v1.x, v2.y, v1.z);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(v1.x, v1.y, v1.z); glVertex3f(v1.x, v1.y, v2.z);
    glVertex3f(v2.x, v1.y, v1.z); glVertex3f(v2.x, v1.y, v2.z);
    glVertex3f(v2.x, v2.y, v1.z); glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v1.x, v2.y, v1.z); glVertex3f(v1.x, v2.y, v2.z);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(v1.x, v1.y, v2.z);
    glVertex3f(v2.x, v1.y, v2.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v1.x, v2.y, v2.z);
    glEnd();
}

void fill_box(Box box)
{
    Vector v1 = box.min;
    Vector v2 = box.max;

    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v1.x, v1.y, v2.z);
    glVertex3f(v1.x, v2.y, v2.z);
    glVertex3f(v1.x, v2.y, v2.z);

    glNormal3f(+1, 0, 0);
    glVertex3f(v2.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v2.x, v1.y, v2.z);

    glNormal3f(0, -1, 0);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v1.y, v1.z);
    glVertex3f(v2.x, v1.y, v2.z);
    glVertex3f(v1.x, v1.y, v2.z);

    glNormal3f(0, +1, 0);
    glVertex3f(v1.x, v2.y, v1.z);
    glVertex3f(v1.x, v2.y, v2.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v2.x, v2.y, v1.z);

    glNormal3f(0, 0, -1);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v1.x, v2.y, v1.z);
    glVertex3f(v2.x, v2.y, v1.z);
    glVertex3f(v2.x, v1.y, v1.z);

    glNormal3f(0, 0, +1);
    glVertex3f(v1.x, v1.y, v2.z);
    glVertex3f(v2.x, v1.y, v2.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v1.x, v2.y, v2.z);
    glEnd();
}

void draw_samples(Vector const samples[], int sample_count)
{
    glBegin(GL_POINTS);
    for (int i = 0; i != sample_count; ++ i)
    {
        vector_apply(samples[i]);
    }
    glEnd();
}

Array * function_array(Function f, float a, float b, int n)
{
    int n1 = n + 1;
    int i;

    Box box = MIN_BOX;
    Vector * vertices = malloc_array(Vector, n1);

    for (i = 0; i <= n; ++ i)
    {
        float t = (float) i / n;
        float x = interpolate(a, b, t);
        float y = f(x);

        Vector vertex = {x, y, 0};
        box = box_add(box, vertex);
        vertices[i] = vertex;

        vector_print(vertex); puts("");
    }

    Array * array = array_new(GL_LINE_STRIP);
    array->vertex_count = n1;
    array->vertices = vertices;

    return array;
}

Array * spherical_function_array(Spherical_Function f, int nu, int nv)
{
    int vertex_count = nu * (nv + 1);
    int face_count = nu * nv;
    int i, j, k = 0;

    Box box = MIN_BOX;
    Vector * vertices = malloc_array(Vector, vertex_count);
    Color * colors = malloc_array(Color, vertex_count);
    Quad * quads = malloc_array(Quad, face_count);

    for (i = 0; i <= nv; ++ i)
    {
        float v = (float) i / nv;
        float theta = v * M_PI - M_PI/2;

        for (j = 0; j != nu; ++ j)
        {
            float u = (float) j / nu;
            float phi = u * 2 * M_PI;
            float radius = f(phi, theta);

            Vector vertex = vector_spheric(phi, theta, radius);
            box = box_add(box, vertex);

            vertices[k] = vertex;
            colors[k] = color_wheel_orgb(phi);

            if (i != nv)
            {
                Quad * quad = &quads[k];
                quad->a = i * nu + j;
                quad->b = i * nu + ((j + 1) == nu ? 0 : j + 1);
                quad->c = (i + 1) * nu + ((j + 1) == nu ? 0 : j + 1);
                quad->d = (i + 1) * nu + j;
            }

            ++ k;
        }
    }

    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count = face_count;
    array->vertices = vertices;
    array->colors = colors;
    array->faces.quads = quads;

    array_add_normals(array);

    return array;
}

Array * hemisphere_array(void * data, Spherical_Function_W f, int nu, int nv)
{
    int vertex_count = nu * (nv + 1);
    int face_count = nu * nv;
    int i, j, k = 0;

    Vector * vertices = malloc_array(Vector, vertex_count);
    Color * colors = malloc_array(Color, vertex_count);
    Quad * quads = malloc_array(Quad, face_count);

    for (i = 0; i <= nv; ++ i)
    {
        float v = (float) i / nv;
        float theta = v * M_PI/2;

        for (j = 0; j != nu; ++ j)
        {
            float u = (float) j / nu;
            float phi = u * 2 * M_PI;
            Vector w = vector_spheric(phi, theta, 1);
            float radius = f(data, w);

            Vector vertex = vector_spheric(phi, theta, radius);

            vertices[k] = vertex;
//            colors[k] = color_wheel_orgb(phi);
            colors[k] = color_wheel_hsv(phi);

            if (i != nv)
            {
                Quad * quad = &quads[k];
                quad->a = i * nu + j;
                quad->b = i * nu + ((j + 1) == nu ? 0 : j + 1);
                quad->c = (i + 1) * nu + ((j + 1) == nu ? 0 : j + 1);
                quad->d = (i + 1) * nu + j;
            }

            ++ k;
        }
    }

    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count = face_count;
    array->vertices = vertices;
    array->colors = colors;
    array->faces.quads = quads;

    array_add_normals(array);

    return array;
}

#if 0
Array * spherical_function_array_2(Spherical_Function f, int nu, int nv)
{
    assert(nu >= 3);
    assert(nv >= 2);

    int vertex_count = nu * (nv - 1) + 2;
    int face_count = nu * (nv - 1)
    int i, j, k = 0;

    Box box = MIN_BOX;
    Vector * vertices = malloc_array(Vector, vertex_count);
    Color * colors = malloc_array(Color, vertex_count);
    Face * faces = malloc_array(Face, face_count);

    float const angle_offset = M_PI / nu;

    float north_radius = f(0, M_PI / 2);
    Vector vertex = vector_spheric(phi, theta, radius);
    box = box_add(box_vertex);

    // ----

    for (j = 0; j != nu; ++ j)
    {
        float v = (float) j / nu;
        float theta = 1 - (float) (nv / 2 - 1) / nv;

        float radius = f(phi, theta);
    }

    for (i = 1; i < nv; ++ i)
    {
        float v = (float) i / nv;
        float theta = v * M_PI - M_PI/2;

        for (j = 0; j != nu; ++ j)
        {
            float u = (float) j / nu;
            float phi = u * 2 * M_PI;
            float radius = f(phi, theta);

            Vector vertex = vector_spheric(phi, theta, radius);
            box = box_add(box, vertex);

            vertices[k] = vertex;
            colors[k] = color_wheel_orgb(phi);

            if (i != nv)
            {
                Quad * quad = &quads[k];
                quad->a = i * nu + j;
                quad->b = i * nu + ((j + 1) == nu ? 0 : j + 1);
                quad->c = (i + 1) * nu + ((j + 1) == nu ? 0 : j + 1);
                quad->d = (i + 1) * nu + j;
            }

            ++ k;
        }
    }

    Array * array = array_new(GL_TRIANGLES);
    array->box = box;
    array->vertex_count = vertex_count;
    array->face_count = face_count;
    array->vertices = vertices;
    array->colors = colors;
    array->faces = faces;

    array_add_normals(array);

    return array;
}
#endif

float spherical_cube(float phi, float theta)
{
    Vector omega = vector_spheric(phi, theta, 1);
    float max = fmax(fmax(fabs(omega.x), fabs(omega.y)), fabs(omega.z));
    return  1 / max;
}

float spherical_sphere(float phi, float theta)
{
    return 1;
}

float spherical_test(float phi, float theta)
{
    theta = M_PI/2 - theta;

    return fmax(0, 5 * cos(theta) - 4) + fmax(0, -4 * sin(theta - M_PI) * cos(phi - 2.5) - 3);
}

Array * array_cube(void)
{
    Array * array = array_new(GL_QUADS);

    static Quad const cube_faces[6] =
    {
        {0, 1, 5, 4},
        {0, 2, 3, 1},
        {0, 4, 6, 2},
        {7, 6, 4, 5},
        {7, 5, 1, 3},
        {7, 3, 2, 6}
    };

#if 1
    array->vertices = copy_array(Vector, unit_cube_vertices, 8);
    array->vertex_count = 8;
#else
    static Vector const cube_vertices2[6][4] =
    {
        {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},
        {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}},
        {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},

        {{1, 1, 1}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}},
        {{1, 1, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}},
        {{1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}}
    };

    static Vector const cube_normals[6][4] =
    {
        {{0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}},
        {{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}},
        {{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}},
        {{0, 0, +1}, {0, 0, +1}, {0, 0, +1}, {0, 0, +1}},
        {{+1, 0, 0}, {+1, 0, 0}, {+1, 0, 0}, {+1, 0, 0}},
        {{0, +1, 0}, {0, +1, 0}, {0, +1, 0}, {0, +1, 0}},
    };

    array->vertices = copy_array(Vector, cube_vertices2, 6 * 4);
    array->normals  = copy_array(Vector, cube_normals,   6 * 4);
    array->vertex_count = 6 * 4;
#endif

    array->faces.quads = copy_array(Quad, cube_faces, 6 * 4);
    array->face_count = 6;

    return array;
}

Array * array_arc(float radius, float angle, int n)
{
    Vector * vertices = malloc_array(Vector, n);
    Vector * normals  = malloc_array(Vector, n);

    for (int i = 0; i != n; ++ i)
    {
        float t = (float) i / n;
        float phi = t * angle;

        vertices[i] = vector_polar(phi, radius);
        normals[i]  = vector_polar(phi, 1.0);
    }

    Array * array = array_new(GL_LINE_STRIP);
    array->vertices = vertices;
    array->normals  = normals;
    array->vertex_count = n;

    return array;
}

Array * array_circle(float radius, int n)
{
    Vector * vertices = malloc_array(Vector, n);
    Vector * normals  = malloc_array(Vector, n);

    for (int i = 0; i != n; ++ i)
    {
        float t = (float) i / n;
        float phi = t * 2 * M_PI;

        vertices[i] = vector_polar(phi, radius);
        normals[i]  = vector_polar(phi, 1.0);
    }

    Array * array = array_new(GL_LINE_LOOP);
    array->vertices = vertices;
    array->normals  = normals;
    array->vertex_count = n;

    return array;
}

Array * array_regular_polygon(float radius, int n)
{
    Vector * vertices = malloc_array(Vector, 2 * n);
    Vector * normals  = malloc_array(Vector, 2 * n);

    for (int i = 0; i != n; ++ i)
    {
        float t_0 = (float) (i + 0) / n;
        float t_1 = (float) (i + 1) / n;

        float phi_0 = t_0 * 2 * M_PI;
        float phi_1 = t_1 * 2 * M_PI;

        vertices[2 * i + 0] = vector_polar(phi_0, radius);
        vertices[2 * i + 1] = vector_polar(phi_1, radius);

        normals[2 * i + 0]  = vector_polar((phi_0 + phi_1) / 2, 1.0);
        normals[2 * i + 1]  = vector_polar((phi_0 + phi_1) / 2, 1.0);
    }

    Array * array = array_new(GL_LINES);
    array->vertices = vertices;
    array->normals  = normals;
    array->vertex_count = 2 * n;

    return array;
}

Array * array_disc(Vector center, float radius, int n)
{
    Vector * vertices = malloc_array(Vector, n + 2);

    vertices[0] = center;

    for (int i = 0; i != n; ++ i)
    {
        float t = (float) i / n;
        float phi = t * 2 * M_PI;

        vertices[1 + i] = vector_polar(phi, radius);
    }

    vertices[n + 1] = vertices[1];

    Array * array = array_new(GL_TRIANGLE_FAN);
    array->vertices = vertices;
    array->vertex_count = n + 2;

    return array;
}

#if 0
void draw_arrow(Vector position, Vector direction, float length, int n1, int n2)
{
    Vector tip = vector_add_scaled(position, direction, length);
    Vector middle = vector_add_scaled(position, direction, length / 2);

    int min_dim = vector_min_dim(direction);
    Vector turner = min_dim == 0 ? VECTOR_X : min_dim == 1 ? VECTOR_Y : VECTOR_Z;
    Vector x = cross(turner, direction);
    Vector y = cross(direction, x);
    Vector d = direction;
    Vector p = position;

    Matrix m =
    {
        {
            {x.x, x.y, x.z, 0},
            {y.x, y.y, y.z, 0},
            {d.x, d.y, d.z, 0},
            {p.x, p.y, p.z, 1}
        }
    };

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i != n1; ++ i)
    {
        float t = (float() i / n2;
        float angle = t * 2 * M_PI;

        Vector v = vector_polar(angle, length / 2);) 
        v = matrix_mul(v);
        vector_apply(v);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    vector_apply(tip);
    for (int i = 0; i <= n2; ++ i)
    {
        float t = (float() i / n2;
        float angle = t * 2 * M_PI;

        Vector v = vector_polar(angle, length / 2);) 
        v = matrix_mul(v);
        vector_apply(v);
    }
    glEnd();
}
#endif

void draw_disk(float radius, int n)
{
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(0, 0);

    for (int i = 0; i <= n; ++ i)
    {
        float phi = interpolate(0, 2 * M_PI, (float) i / n);
        glVertex2f(radius * cos(phi), radius * sin(phi));
    }

    glEnd();
}

void draw_cylinder(Cylinder cylinder, int n)
{
    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i <= n; ++ i)
    {
        float t = (float) i / n;
        float phi = interpolate(0, cylinder.max_phi, t);
        float x = cylinder.radius * cos(phi);
        float y = cylinder.radius * sin(phi);

        glVertex3f(x, y, cylinder.min_z);
        glVertex3f(x, y, cylinder.max_z);
    }

    glEnd();
}

void draw_cylinder_closed(Cylinder cylinder, int n)
{
    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i <= n; ++ i)
    {
        float t = (float) i / n;
        float phi = interpolate(0, cylinder.max_phi, t);
        float x = cylinder.radius * cos(phi);
        float y = cylinder.radius * sin(phi);

        glVertex3f(x, y, cylinder.min_z);
        glVertex3f(x, y, cylinder.max_z);
    }

    glEnd();

    //draw_disk_2(vector(0, 0, cylinder.min_z), cylinder.radius, n);
    //draw_disk_2(vector(0, 0, cylinder.max_z), cylinder.radius, n);
}

Matrix matrix_transform_base(Vector z, Vector t)
{
    z = vector_normalize(z);

    Vector x, y;
    vector_orthogonals(z, &x, &y);

    Matrix m =
    {
        {
            {x.x, x.y, x.z, 0.0},
            {y.x, y.y, y.z, 0.0},
            {z.x, z.y, z.z, 0.0},
            {t.x, t.y, t.z, 1.0}
        }
    };

    return m;
}

void draw_arrow(Vector from, Vector to, int n)
{
    glBegin(GL_LINES);
    vector_apply(from);
    vector_apply(to);
    glEnd();

    float head_length = 0.2;
    float head_radius = 0.05;

    Vector z = vector_sub(to, from);
    float length = vector_length(z);
    Vector p = vector_add_scaled(from, z, (1.0 - head_length));
    Matrix transform = matrix_transform_base(z, p);

    glBegin(GL_TRIANGLE_FAN);
    vector_apply(to);
    for (int i = 0; i <= n; ++ i)
    {
        float t = (float) i / n;
        float phi = 2 * M_PI * t;

        Vector v = vector_polar(phi, head_radius * length);
        v = matrix_mul(transform, v);
        vector_apply(v);
    }
    glEnd();
}

static void draw_coordinates_scale(float scale, Font_ * font)
{
    Vector position_x = vector_scale(VECTOR_X, scale);
    Vector position_y = vector_scale(VECTOR_Y, scale);
    Vector position_z = vector_scale(VECTOR_Z, scale);

    color_apply(RED);   draw_arrow(ORIGIN, position_x, 16);
    color_apply(GREEN); draw_arrow(ORIGIN, position_y, 16);
    color_apply(BLUE);  draw_arrow(ORIGIN, position_z, 16);
}

void draw_coordinates(float scale)
{
    glLineWidth(3.0);
    draw_coordinates_scale(scale, NULL);
    glLineWidth(1.0);
}

void draw_chess_board(void)
{
    float d = (float) 2 / 8;

    glBegin(GL_QUADS);

    for (int i = 0; i <= 8; ++ i)
    {
        float v = (float) i / 8;
        float y = interpolate(-1, 1, v);

        for (int j = 0; j != 8; ++ j)
        {
            float u = (float) j / 8;
            float x = interpolate(-1, 1, u);

            color_apply(((i + j) % 2) == 1 ? WHITE : BLACK);

            glVertex2f(x, y);
            glVertex2f(x + d, y);
            glVertex2f(x + d, y + d);
            glVertex2f(x, y + d);
        }
    }

    glEnd();
}

#if 0
static Vector homogenize(Vector v)
{
    if (v.z == 0.0)
        return v;

    v.x = v.x / v.z;
    v.y = v.y / v.z;
    v.z = 1.0;

    return v;
}

void draw_thick_lines(Vector vertices[], int count, float d)
{
    glBegin(GL_TRIANGLE_STRIP);
    vector_apply(vertices[0]);
    vector_apply(vertices[0]);
    for (int i = 1; i < count - 1; ++ i)
    {
        Vector v1 = vertices[i-1];
        Vector v2 = vertices[i];
        Vector v3 = vertices[i+1];
        Vector a = vector_sub(v2, v1);
        Vector b = vector_sub(v3, v2);
        Vector na = vector_normalize(vector(-a.y, a.x, 0)); na.z = -dot(na, v2);
        Vector nb = vector_normalize(vector(-b.y, b.x, 0)); na.z = -dot(nb, v2);
        Vector l = cross(vector_add(na, vector(0, 0,+d)), vector_add(nb, vector(0, 0,+d)));
        Vector r = cross(vector_add(na, vector(0, 0,-d)), vector_add(nb, vector(0, 0,-d)));
        l = homogenize(l); l.z = 0;
        r = homogenize(r); r.z = 0;
        vector_apply(r);
        vector_apply(l);
    }
    vector_apply(vertices[count - 1]);
    vector_apply(vertices[count - 1]);
    glEnd();

    color_apply(BLACK);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i != count; ++ i)
    {
        vector_apply(vertices[i]);
    }
    glEnd();
}
#endif

Array * array_revolve(Array const * source, Vector axis, int n)
{
    Size size = {n, source->vertex_count, 1};
    int vertex_count = size_total(size);
    int face_count   = size_total(size);

    Vector const * source_vertices   = source->vertices;
    Vector const * source_normals    = source->normals;
    Vector const * source_tex_coords = source->tex_coords;

    Vector       * vertices   = malloc_array(Vector, vertex_count);
    Vector       * normals    = source_normals    ? malloc_array(Vector, vertex_count) : NULL;
    Vector       * tex_coords = source_tex_coords ? malloc_array(Vector, vertex_count) : NULL;
    Quad         * quads      = malloc_array(Quad, face_count);

    for (int i = 0; i != size.y; ++ i)
    for (int j = 0; j != size.x; ++ j)
    {
        int index = size_index(size, 0, i, j);
        float phi = 2.0 * M_PI * j / n;
        Quaternion q = quaternion_from_vector(axis, phi);

        vertices[index] = quaternion_rotate(q, source_vertices[i]);
        if (normals)
            normals[index]  = quaternion_rotate(q, source_normals[i]);
        if (tex_coords)
        {
            Vector coord = tex_coords[i];
            coord.x = (float) j / size.x;
            tex_coords[index] = tex_coords[i];
        }

        Quad quad =
        {
            size_index(size, 0, (i + 0) % size.y, (j + 0) % size.x),
            size_index(size, 0, (i + 0) % size.y, (j + 1) % size.x),
            size_index(size, 0, (i + 1) % size.y, (j + 1) % size.x),
            size_index(size, 0, (i + 1) % size.y, (j + 0) % size.x)
        };
        quads[index] = quad;
    }
    
    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count   = face_count;
    array->vertices     = vertices;
    array->normals      = normals;
    array->tex_coords   = tex_coords;
    array->faces.quads  = quads;

    return array;
}

Array * array_extrude(Array const * source, Vector direction)
{
    Size size = {source->vertex_count, 2, 1};
    int vertex_count = source->vertex_count * 2;
    int face_count   = source->vertex_count;

    Vector const * source_vertices = source->vertices;
    Vector const * source_normals  = source->normals;
    Vector       * vertices = malloc_array(Vector, vertex_count);
    Vector       * normals  = malloc_array(Vector, vertex_count);
    Quad         * quads    = malloc_array(Quad,   source->vertex_count);

    memcpy(vertices, source_vertices, sizeof(Vector) * source->vertex_count);
    memcpy(normals,  source_normals,  sizeof(Vector) * source->vertex_count);
    memcpy(&normals[source->vertex_count],  source_normals, sizeof(Vector) * source->vertex_count);

    for (int i = 0; i != size.x; ++ i)
    {
        vertices[source->vertex_count + i] = vector_add(source_vertices[i], direction);

        Quad q = {
            size_index(size, 0, 0, i),
            size_index(size, 0, 0, (i + 1) % size.x),
            size_index(size, 0, 1, (i + 1) % size.x),
            size_index(size, 0, 1, i)
        };
        quads[i] = q;
    }

    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count   = face_count;
    array->vertices     = vertices;
    array->normals      = normals;
    array->faces.quads  = quads;

    return array;
}

Array * array_extrude_2(Array const * source, Parametric_Curve f, void const * data, int segments)
{
    Size size = {source->vertex_count, segments, 1};
    int vertex_count = source->vertex_count * segments;
    int face_count   = source->vertex_count * segments;

    Vector const * source_vertices = source->vertices;
    Vector const * source_normals  = source->normals;
    Color  const * source_colors   = source->colors;
    Vector       * vertices = malloc_array(Vector, vertex_count);
    Vector       * normals  = source_normals ? malloc_array(Vector, vertex_count) : NULL;
    Color        * colors   = source_colors  ? malloc_array(Color,  vertex_count) : NULL;
    Quad         * quads    = malloc_array(Quad,   source->vertex_count * segments);

    Vector a = f(data, 0);

    for (int j = 0; j < segments; ++ j)
    {
        float t = (float) j / segments;
        float delta = (float) 1.0 / segments;

        Vector b = f(data, t);

        Vector direction = vector_normalize(vector_sub(f(data, t + delta), (f(data, t - delta))));
        Vector up = VECTOR_Z;
        Vector side = cross(direction, up);
        up = cross(side, direction);

        Vector ex = side;
        Vector ey = up;
        Vector ez = direction;

        for (int i = 0; i != size.x; ++ i)
        {
            Vector p_source = source_vertices[i];
            Vector p_target = vector_add(b, vector_transform(ex, ey, ez, p_source));
            vertices[j * source->vertex_count + i] = p_target;

            if (normals)
            {
#if 0
                Vector n_source = vector_scale(vector_add(source_normals[i], source_normals[(i + 1) % source->vertex_count]), 0.5);
                // hmph
#endif
                Vector n_source = source_normals[i];
                Vector n_target = vector_transform(ex, ey, ez, n_source);
                normals[j * source->vertex_count + i] = vector_normalize(n_target);
            }
          
            if (colors)
                colors[j * source->vertex_count + i] = source_colors[i];

            Quad q =
            {
                size_index(size, 0, j + 0, i),
                size_index(size, 0, j + 0, (i + 1) % size.x),
                size_index(size, 0, (j + 1) % segments, (i + 1) % size.x),
                size_index(size, 0, (j + 1) % segments, i)
            };
            quads[j * source->vertex_count + i] = q;
        }

        a = b;
    }

    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count   = face_count;
    array->vertices     = vertices;
    array->colors       = colors;
    array->normals      = normals;
    array->faces.quads  = quads;

    return array;
}

Array * array_parametric_curve(Parametric_Curve f, void const * data, int n)
{
    int vertex_count = n;

    Vector * vertices = malloc_array(Vector, vertex_count);

    for (int i = 0; i != n; ++ i)
    {
        float t = (float) i / n;

        vertices[i] = f(data, t);
    }

    Array * array = array_new(GL_LINE_LOOP);
    array->vertex_count = vertex_count;
    array->vertices = vertices;

    return array;
}

Vector rigid_transform(Rigid_Transformation transformation, Vector v)
{
    v = quaternion_rotate(transformation.orientation, v);
    return vector_add(transformation.position, v);
}

Array * array_twist(Array const * source, Flight flight, int n)
{
    Size size = {source->vertex_count, n, 1};
    int vertex_count = size_total(size);
    int face_count   = size_total(size);

    Vector const * source_vertices = source->vertices;
    Vector const * source_normals  = source->normals;

    Vector * vertices = malloc_array(Vector, vertex_count);
    Vector * normals  = malloc_array(Vector, vertex_count);
    Quad   * quads    = malloc_array(Quad,   vertex_count);

    for (int i = 0; i != size.y; ++ i)
    {
        float t = (float) i / n;
        Rigid_Transformation transformation = flight(t);

        for (int j = 0; j != size.x; ++ j)
        {
            int index = size_index(size, 0, i, j);

            vertices[index] = rigid_transform(transformation, source_vertices[j]);
            normals[index]  = quaternion_rotate(transformation.orientation, source_normals[j]);

            Quad quad =
            {
                size_index(size, 0, (i + 0) % size.y, (j + 0) % size.x),
                size_index(size, 0, (i + 0) % size.y, (j + 1) % size.x),
                size_index(size, 0, (i + 1) % size.y, (j + 1) % size.x),
                size_index(size, 0, (i + 1) % size.y, (j + 0) % size.x)
            };
            quads[index] = quad;
        }
    }

    Array * array = array_new(GL_QUADS);
    array->vertex_count = vertex_count;
    array->face_count   = face_count;
    array->vertices     = vertices;
    array->normals      = normals;
    array->faces.quads  = quads;

    return array;
}

void array_translate(Array * array, Vector t)
{
    for (int i = 0; i != array->vertex_count; ++ i)
    {
        array->vertices[i] = vector_add(array->vertices[i], t);
    }
}

void array_scale(Array * array, Vector s)
{
    for (int i = 0; i != array->vertex_count; ++ i)
    {
        array->vertices[i] = vector_mul(array->vertices[i], s);
    }
}

void array_rotate(Array * array, Vector axis, float angle)
{
    Quaternion q = quaternion_from_vector(axis, angle);

    for (int i = 0; i != array->vertex_count; ++ i)
    {
        array->vertices[i] = quaternion_rotate(q, array->vertices[i]);
    }
}

Array * array_torus(float r1, float r2, int n1, int n2)
{
    Array * circle = array_circle(r2, n2);
    array_rotate(circle, VECTOR_X, M_PI / 2);
    array_translate(circle, vector(r1, 0, 0));

    return array_revolve(circle, VECTOR_Z, n1);
}

Array * array_frustum(float r1, float r2, float h, int n)
{
    Array * contour = array_new(GL_LINE_STRIP);
    array_append_vertex(contour, ORIGIN);
    array_append_vertex(contour, vector(r1, 0, 0));
    array_append_vertex(contour, vector(r2, 0, h));
    array_append_vertex(contour, vector( 0, 0, h));

    return array_revolve(contour, VECTOR_Z, n);
}

Array * array_cone(float r, float h, int n)
{
    return array_frustum(r, 0, h, n);
}

Array * array_cylinder(float r, float h, int n)
{
    return array_frustum(r, r, h, n);
}

Array * array_sphere(float r, int nx, int ny)
{
    Array * hemi_circle = array_arc(r, M_PI, ny);
    array_rotate(hemi_circle, VECTOR_Z, - M_PI/2);

    return array_revolve(hemi_circle, VECTOR_Z, nx);
}

Array * array_face_normals(Array const * source)
{
    error_check(source->mode != GL_TRIANGLES, "array_face_normals requires mode == GL_TRIANGLES");

    Vector const * source_vertices  = source->vertices;
    Face   const * source_triangles = source->faces.triangles;

    int face_count   = source->face_count;
    int vertex_count = face_count * 3;

    Vector * vertices = malloc_array(Vector, vertex_count);
    Vector * normals  = malloc_array(Vector, vertex_count);
    Face * triangles  = malloc_array(Face,   face_count);

    for (int i = 0; i != face_count; ++ i)
    {
        Face t1 = source_triangles[i];
        Face t2 = {i * 3 + 0, i * 3 + 1, i * 3 + 2};

        vertices[t2.a] = source_vertices[t1.a];
        vertices[t2.b] = source_vertices[t1.b];
        vertices[t2.c] = source_vertices[t1.c];

        normals[t2.a] =
        normals[t2.b] =
        normals[t2.c] = triangle_normal(source_vertices[t1.a], source_vertices[t1.b], source_vertices[t1.c]);

        triangles[i] = t2;
    }

    Array * array = array_new(GL_TRIANGLES);
    array->vertex_count    = vertex_count;
    array->vertices        = vertices;
    array->normals         = normals;
    array->face_count      = face_count;
    array->faces.triangles = triangles;

    return array;
}

Array * array_tetrahedron(void)
{
    float a = sqrt(8.0 / 3.0);
    float h = a * sqrt(6.0) / 3.0;
    float r = a / (2 * cos(M_PI / 6));

    float c = cos(2.0 * M_PI / 3.0);
    float s = sin(2.0 * M_PI / 3.0);

    Vector vertices[] =
    {
        {0, 0, h},
        {r, 0, 0},
        {r * c, + r * s, 0},
        {r * c, - r * s, 0}
    };

    Face triangles[] =
    {
        {0, 1, 2},
        {0, 2, 3},
        {0, 3, 1},
        {3, 2, 1}
    };

    Array * array = array_new(GL_TRIANGLES);
    array->vertices        = copy_array(Vector, vertices,  4);
    array->faces.triangles = copy_array(Face,   triangles, 4);
    array->vertex_count = 4;
    array->face_count = 4;

    array_translate(array, vector(0, 0, 1.0 - h));

    return array_face_normals(array);
}

Array * array_octahedron(void)
{
    Face triangles[] =
    {
        {0, 2, 5},
        {2, 1, 5},
        {1, 3, 5},
        {3, 0, 5},
        {4, 3, 1},
        {4, 0, 3},
        {4, 2, 0},
        {4, 1, 2}
    };

    Array * array = array_new(GL_TRIANGLES);
    array->vertices        = copy_array(Vector, octahedron_vertices, 6);
    array->faces.triangles = copy_array(Face,   triangles, 8);
    array->vertex_count = 6;
    array->face_count = 8;

    return array_face_normals(array);
}

static void icosahedron_vertices(Vector target_vertices[12])
{
    float a = 4.0 / sqrt(10.0 + 2.0 * sqrt(5.0));

    float theta = M_PI/2 - 2 * asin(a/2);
    float r = cos(theta);
    float z = sin(theta);

    float phi1[5], phi2[5];
    for (int i = 0; i != 5; ++ i)
    {
        phi1[i] = (float) (2.0 * M_PI * (i + 0.0)) / 5;
        phi2[i] = (float) (2.0 * M_PI * (i + 0.5)) / 5;
    }

    Vector source_vertices[] =
    {
        {0, 0, -1},
        {r * cos(phi1[0]), r * sin(phi1[0]), -z},
        {r * cos(phi1[1]), r * sin(phi1[1]), -z},
        {r * cos(phi1[2]), r * sin(phi1[2]), -z},
        {r * cos(phi1[3]), r * sin(phi1[3]), -z},
        {r * cos(phi1[4]), r * sin(phi1[4]), -z},
        {r * cos(phi2[0]), r * sin(phi2[0]), +z},
        {r * cos(phi2[1]), r * sin(phi2[1]), +z},
        {r * cos(phi2[2]), r * sin(phi2[2]), +z},
        {r * cos(phi2[3]), r * sin(phi2[3]), +z},
        {r * cos(phi2[4]), r * sin(phi2[4]), +z},
        {0, 0, +1},
    };

    memcpy(target_vertices, source_vertices, 12 * sizeof(Vector));
}

static void icosahedron_triangles(Face target_triangles[20])
{
    Face source_triangles[] =
    {
        { 0,  2,  1},
        { 0,  3,  2},
        { 0,  4,  3},
        { 0,  5,  4},
        { 0,  1,  5},
        { 1,  2,  6},
        { 2,  3,  7},
        { 3,  4,  8},
        { 4,  5,  9},
        { 5,  1, 10},
        { 1,  6, 10},
        { 2,  7,  6},
        { 3,  8,  7},
        { 4,  9,  8},
        { 5, 10,  9},
        {11,  6,  7},
        {11,  7,  8},
        {11,  8,  9},
        {11,  9, 10},
        {11, 10,  6},
    };

    memcpy(target_triangles, source_triangles, 20 * sizeof(Face));
}

Array * array_icosahedron(void)
{
    Vector vertices[12];
    icosahedron_vertices(vertices);

    Face triangles[20];
    icosahedron_triangles(triangles);

    Array * array = array_new(GL_TRIANGLES);
    array->vertices        = copy_array(Vector, vertices, 12);
    array->faces.triangles = copy_array(Face,   triangles, 20);
    array->vertex_count = 12;
    array->face_count = 20;

    return array_face_normals(array);
}

Array * array_dodecahedron(void)
{
    Vector source_vertices[12];
    icosahedron_vertices(source_vertices);

    Face source_triangles[20];
    icosahedron_triangles(source_triangles);

    int vertex_count = 20;
    Vector vertices[20];
    for (int i = 0; i != 20; ++ i)
    {
        Face t = source_triangles[i];
        Vector sum = vector_add3(source_vertices[t.a], source_vertices[t.b], source_vertices[t.c]);
        vertices[i] = vector_normalize(sum);
    }

    Pentagon pentagons[12] =
    {
        {0, 1, 2, 3, 4},
        {0, 1, 6, 11, 5},
        {1, 2, 7, 12, 6},
        {2, 3, 8, 13, 7},
        {3, 4, 9, 14, 8},
        {4, 0, 5, 10, 9},
        {11, 6, 12, 16, 15},
        {12, 7, 13, 17, 16},
        {13, 8, 14, 18, 17},
        {14, 9, 10, 19, 18},
        {10, 5, 11, 15, 19},
        {15, 16, 17, 18, 19}
    };

    int face_count = 12 * 3;
    Face triangles[12 * 3];
    for (int i = 0; i != 12; ++ i)
    {
        Pentagon p = pentagons[i];
        Face t1 = {p.a, p.b, p.c};
        Face t2 = {p.a, p.c, p.d};
        Face t3 = {p.a, p.d, p.e};

        triangles[i * 3 + 0] = t1;
        triangles[i * 3 + 1] = t2;
        triangles[i * 3 + 2] = t3;
    }

    Array * array = array_new(GL_TRIANGLES);
    array->vertices        = copy_array(Vector, vertices, vertex_count);
    array->faces.triangles = copy_array(Face,   triangles, face_count);
    array->vertex_count = vertex_count;
    array->face_count = face_count;

    return array_face_normals(array);
}

