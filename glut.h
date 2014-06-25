#ifndef GLUT_H
#define GLUT_H

#ifdef DARWIN
#include <GLUT/glut.h>
#else
#include "opengl.h"
#include <GL/glut.h>
#endif

#define GLUT_SCROLL_UP   (3)
#define GLUT_SCROLL_DOWN (4)

#endif
