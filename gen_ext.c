#include <string.h>
#include "opengl.h"

int pGL_ARB_texture_non_power_of_two;
int pGL_ARB_vertex_buffer_object;
int pGL_EXT_framebuffer_object;

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
PFNGLACTIVETEXTUREPROC pglActiveTexture;
PFNGLATTACHSHADERPROC pglAttachShader;
PFNGLBINDBUFFERPROC pglBindBuffer;
PFNGLBINDFRAMEBUFFEREXTPROC pglBindFramebufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC pglBindRenderbufferEXT;
PFNGLBUFFERDATAPROC pglBufferData;
PFNGLCOMPILESHADERPROC pglCompileShader;
PFNGLCREATEPROGRAMPROC pglCreateProgram;
PFNGLCREATESHADERPROC pglCreateShader;
PFNGLDELETEBUFFERSPROC pglDeleteBuffers;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC pglFramebufferRenderbufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC pglFramebufferTexture2DEXT;
PFNGLGENBUFFERSPROC pglGenBuffers;
PFNGLGENFRAMEBUFFERSEXTPROC pglGenFramebuffersEXT;
PFNGLGETATTRIBLOCATIONPROC pglGetAttribLocation;
PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC pglGetProgramiv;
PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog;
PFNGLGETSHADERIVPROC pglGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation;
PFNGLLINKPROGRAMPROC pglLinkProgram;
PFNGLRENDERBUFFERSTORAGEEXTPROC pglRenderbufferStorageEXT;
PFNGLSHADERSOURCEPROC pglShaderSource;
PFNGLTEXIMAGE3DPROC pglTexImage3D;
PFNGLTEXSUBIMAGE3DPROC pglTexSubImage3D;
PFNGLUNIFORM1FPROC pglUniform1f;
PFNGLUNIFORM1IPROC pglUniform1i;
PFNGLUNIFORM2FVPROC pglUniform2fv;
PFNGLUNIFORM3FPROC pglUniform3f;
PFNGLUNIFORM3FVPROC pglUniform3fv;
PFNGLUNIFORM4FVPROC pglUniform4fv;
PFNGLUNIFORMMATRIX4FVPROC pglUniformMatrix4fv;
PFNGLUSEPROGRAMPROC pglUseProgram;
PFNGLVERTEXATTRIB1FPROC pglVertexAttrib1f;
#endif

void glext_init(void)
{
    static int initialized = 0;
    if (initialized)
        return;

    initialized = 1;

    char const * const extensions = (char *) glGetString(GL_EXTENSIONS);

    pGL_ARB_texture_non_power_of_two = strstr(extensions, "GL_ARB_texture_non_power_of_two") != 0;
    pGL_ARB_vertex_buffer_object = strstr(extensions, "GL_ARB_vertex_buffer_object") != 0;
    pGL_EXT_framebuffer_object = strstr(extensions, "GL_EXT_framebuffer_object") != 0;

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
    glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
    glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
    glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
    glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) wglGetProcAddress("glBindFramebufferEXT");
    glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) wglGetProcAddress("glBindRenderbufferEXT");
    glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
    glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
    glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
    glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
    glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
    glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
    glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) wglGetProcAddress("glGenFramebuffersEXT");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
    glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
    glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
    glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) wglGetProcAddress("glRenderbufferStorageEXT");
    glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
    glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) wglGetProcAddress("glTexSubImage3D");
    glUniform1f = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
    glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
    glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress("glUniform2fv");
    glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
    glUniform3fv = (PFNGLUNIFORM3FVPROC) wglGetProcAddress("glUniform3fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC) wglGetProcAddress("glUniform4fv");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
    glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC) wglGetProcAddress("glVertexAttrib1f");
#endif
}
