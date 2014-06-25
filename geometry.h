#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "array.h"
#include "box.h"
#include "primitives.h"
#include "parametric_curve.h"
#include "quaternion.h"
#include "vector.h"

typedef struct {Vector position; Quaternion orientation;} Rigid_Transformation;

typedef Rigid_Transformation (* Flight)(float);
Vector rigid_transform(Rigid_Transformation, Vector);

typedef float (* Function)(float);
typedef float (* Spherical_Function)(float phi, float theta);
typedef float (* Spherical_Function_W)(void const *, Vector);

extern Vector const
    tetrahedron_vertices[4],
    cube_vertices[8],
    unit_cube_vertices[8],
    octahedron_vertices[6],
    cuboctahedron_vertices[12];

void draw_circle(Vector position, float radius, int n);
void fill_circle(Vector position, float radius, int n);
void draw_ellipse(Vector position, float a, float b, int n);
void draw_sphere(Vector position, float radius, int nx, int ny);
void draw_quad(Vector const quad[4]);
void draw_moebius_strip(float radius, float width, int resolution);
void draw_moebius_contour(float radius, float width, int resolution);
void draw_grid(Vector v1, Vector v2, Vector c, Vector d);
void draw_polar_grid(Vector position, float radius, int n_rotational, int n_radial, int resolution);
void draw_rose(int n, int d, float delta);
void draw_torus(float r1, float r2, int nx, int ny);
void draw_sky_box(Box);
void draw_box(Box);
void fill_box(Box);
void draw_samples(Vector const samples[], int sample_count);
void draw_rect(Box);
void fill_rect(Box);
void draw_rect_rounded(Box box, float radius, int n);
void fill_rect_rounded(Box box, float radius, int n);
//void draw_arrow(Vector position, Vector direction);
void draw_cylinder(Cylinder, int n);
void draw_disk(float radius, int n);

Array * torus_array(Vector position, float r1, float r2, int nx, int ny);
Array * function_array(Function, float a, float b, int n);
Array * spherical_function_array(Spherical_Function, int nx, int ny);
Array * hemisphere_array(void * data, Spherical_Function_W f, int nu, int nv);
Array * array_arc(float radius, float angle, int n);
Array * array_circle(float radius, int n);
Array * array_regular_polygon(float radius, int n);
Array * array_disc(Vector center, float radius, int n);
Array * array_revolve(Array const * array, Vector axis, int n);
Array * array_extrude(Array const * array, Vector delta);
Array * array_twist(Array const * array, Flight, int n);
Array * array_extrude_2(Array const * source, Parametric_Curve, void const *, int n);
Array * array_parametric_curve(Parametric_Curve, void const *, int n);

Array * array_torus(float r1, float r2, int nx, int ny);
Array * array_cone(float r, float h, int n);

Array * array_tetrahedron(void);
Array * array_cube(void);
Array * array_octahedron(void);
Array * array_dodecahedron(void);
Array * array_icosahedron(void);

Array * array_face_normals(Array const *);

void array_translate(Array *, Vector);
void array_rotate(Array *, Vector, float);
void array_scale(Array *, Vector);

float spherical_cube(float phi, float theta);
float spherical_sphere(float phi, float theta);
float spherical_test(float phi, float theta);

void draw_coordinates(float scale);
void draw_arrow(Vector from, Vector to, int n);
void draw_chess_board(void);

void draw_thick_lines(Vector vertices[], int count, float d);

#endif
