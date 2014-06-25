#if defined(DARWIN)

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "gen_ext.h"

#elif defined(WINDOWS)

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <wingdi.h>
#include "gen_ext.h"

#elif defined(CYGWIN)

#include <GL/gl.h>
#include <GL/glu.h>
#include "gen_ext.h"
#include <windows.h>
#include <wingdi.h>

#else

// suppress inclusion of system glext.h
#define GL_GLEXT_LEGACY 1

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "gen_ext.h"

#define wglGetProcAddress(foo) glXGetProcAddress((GLubyte const *) foo)

// seems to be unused
#define GL_COMPLEX_CK (0x140D)

#endif

