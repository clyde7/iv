#include <stdio.h>

#include "glut.h"
#include "math_.h"
#include "viewport.h"

Viewport const DEFAULT_VIEWPORT = {0, 0, 512, 512};

void viewport_print(Viewport viewport)
{
    printf("%d, %d, %d, %d", viewport.x, viewport.y, viewport.width, viewport.height);
}

void viewport_apply(Viewport viewport)
{
    glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

Screen_ viewport_to_screen(Viewport viewport, float fov, int max)
{
    float aspect_ratio = (float) viewport.width / viewport.height;
    float arg0 = fov / 2;
    float d = tan(arg0);

    if (max ^ (aspect_ratio < 1))
    {
        float arg2 = atan(d / aspect_ratio);
        Screen_ screen = {-arg0, +arg0, -arg2, +arg2};
        return screen;
    }
    else
    {
        float arg1 = atan(d * aspect_ratio);
        Screen_ screen = {-arg1, +arg1, -arg0, +arg0};
        return screen;
    }
}

Viewport viewport_fit(Viewport viewport, float inner_aspect_ratio)
{
    float outer_aspect_ratio = (float) viewport.width / viewport.height;

    if (inner_aspect_ratio > outer_aspect_ratio)
    {
        Viewport v = viewport;
        v.height = viewport.width / inner_aspect_ratio;
        v.y += (viewport.height - v.height) / 2;

        return v;
    }

    if (inner_aspect_ratio < outer_aspect_ratio)
    {
        Viewport v = viewport;
        v.width = viewport.height * inner_aspect_ratio;
        v.x += (viewport.width - v.width) / 2;

        return v;
    }

    return viewport;
}

Vector viewport_to_camera(Viewport viewport, int x, int y)
{
    Vector v =
    {
        (float) (x - viewport.x) / viewport.width,
        (float) (viewport.height - 1 - y) / viewport.height,
        0
    };
    // --->Y
    //     viewport
    // I need the window viewport!!!

    return v;
}

void viewport_enter_raster(Viewport viewport)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport.width, 0, viewport.height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void viewport_leave_raster(void)
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

Viewport viewport_interpolate(Viewport a, Viewport b, float t)
{
    float x1 = interpolate(a.x, b.x, t);
    float y1 = interpolate(a.y, b.y, t);
    float x2 = interpolate(a.x + a.width, b.x + b.width, t);
    float y2 = interpolate(a.y + a.height, b.y + b.height, t);

    Viewport viewport =
    {
        floor(x1),
        floor(y1),
        floor(x2 - x1),
        floor(y2 - y1)
    };

    return viewport;
}

static Viewport window_get_current(void)
{
    Viewport window =
    {
        glutGet(GLUT_WINDOW_X),
        glutGet(GLUT_WINDOW_Y),
        glutGet(GLUT_WINDOW_WIDTH),
        glutGet(GLUT_WINDOW_HEIGHT)
    };

    return window;
}

void window_toggle_fullscreen(void)
{
    static Viewport old_window;
    static int fullscreen;

    fullscreen = ! fullscreen;

    if (fullscreen)
    {
        old_window = window_get_current();
        glutFullScreen();
    }
    else
    {
        glutReshapeWindow(old_window.width, old_window.height);
    }
}

Matrix viewport_matrix(Viewport viewport)
{
    float x0 = viewport.x;
    float x1 = viewport.x + viewport.width;
    float y0 = viewport.y;
    float y1 = viewport.y + viewport.height;

    Matrix m = 
    {
        {
            {(x1 - x0) / 2.0, 0, 0, 0},
            {0, (y1 - y0) / 2.0, 0, 0},
            {0, 0, 1/2.0, 0},
            {(x0 + x1) / 2.0, (y0 + y1) / 2.0, 1 / 2.0, 1}
        }
    };

    return m;
}

