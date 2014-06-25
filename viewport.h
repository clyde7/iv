#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "matrix.h"
#include "opengl.h"
#include "screen.h"

typedef struct {GLint x, y; GLsizei width, height;} Viewport;
typedef struct {int width, height;} Resolution;

extern Viewport const DEFAULT_VIEWPORT;

void     viewport_print(Viewport);
void     viewport_apply(Viewport);
Screen_  viewport_to_screen(Viewport, float fov, int max);
Vector   viewport_to_camera(Viewport, int x, int y);
Viewport viewport_fit(Viewport, float aspect_ratio);
Viewport viewport_interpolate(Viewport, Viewport, float);
void     viewport_enter_raster(Viewport);
void     viewport_leave_raster(void);
Matrix   viewport_matrix(Viewport);

void window_toggle_fullscreen(void);

int aspect_ratio_from_name(char const name[], float * aspect_ratio);
int resolution_from_name(char const name[], Resolution *);

#endif
