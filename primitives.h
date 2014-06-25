#ifndef PRIMITIVES_H
#define PRIMITIVES_H

typedef struct {Vector center; float radius;} Sphere;
typedef struct {Vector position, normal;} Plane;

typedef struct {float radius;} Disk;
typedef struct {float min_z, max_z, radius, max_phi;} Cylinder;
typedef struct {float r1, r2;} Torus;

#endif
