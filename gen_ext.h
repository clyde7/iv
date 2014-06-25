#ifndef GEN_EXT_H
#define GEN_EXT_H

#include "glext.h"

#ifdef GL_ARB_texture_non_power_of_two
#undef GL_ARB_texture_non_power_of_two
#define GL_ARB_texture_non_power_of_two pGL_ARB_texture_non_power_of_two
#else
#define GL_ARB_texture_non_power_of_two (0)
#endif

#ifdef GL_ARB_vertex_buffer_object
#undef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object pGL_ARB_vertex_buffer_object
#else
#define GL_ARB_vertex_buffer_object (0)
#endif

#ifdef GL_EXT_framebuffer_object
#undef GL_EXT_framebuffer_object
#define GL_EXT_framebuffer_object pGL_EXT_framebuffer_object
#else
#define GL_EXT_framebuffer_object (0)
#endif

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
#define glActiveTexture pglActiveTexture
#define glAttachShader pglAttachShader
#define glBindBuffer pglBindBuffer
#define glBindFramebufferEXT pglBindFramebufferEXT
#define glBindRenderbufferEXT pglBindRenderbufferEXT
#define glBufferData pglBufferData
#define glCompileShader pglCompileShader
#define glCreateProgram pglCreateProgram
#define glCreateShader pglCreateShader
#define glDeleteBuffers pglDeleteBuffers
#define glFramebufferRenderbufferEXT pglFramebufferRenderbufferEXT
#define glFramebufferTexture2DEXT pglFramebufferTexture2DEXT
#define glGenBuffers pglGenBuffers
#define glGenFramebuffersEXT pglGenFramebuffersEXT
#define glGetAttribLocation pglGetAttribLocation
#define glGetProgramInfoLog pglGetProgramInfoLog
#define glGetProgramiv pglGetProgramiv
#define glGetShaderInfoLog pglGetShaderInfoLog
#define glGetShaderiv pglGetShaderiv
#define glGetUniformLocation pglGetUniformLocation
#define glLinkProgram pglLinkProgram
#define glRenderbufferStorageEXT pglRenderbufferStorageEXT
#define glShaderSource pglShaderSource
#define glTexImage3D pglTexImage3D
#define glTexSubImage3D pglTexSubImage3D
#define glUniform1f pglUniform1f
#define glUniform1i pglUniform1i
#define glUniform2fv pglUniform2fv
#define glUniform3f pglUniform3f
#define glUniform3fv pglUniform3fv
#define glUniform4fv pglUniform4fv
#define glUniformMatrix4fv pglUniformMatrix4fv
#define glUseProgram pglUseProgram
#define glVertexAttrib1f pglVertexAttrib1f
#endif
extern int pGL_ARB_texture_non_power_of_two;
extern int pGL_ARB_vertex_buffer_object;
extern int pGL_EXT_framebuffer_object;

void glext_init(void);

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
extern PFNGLACTIVETEXTUREPROC pglActiveTexture;
extern PFNGLATTACHSHADERPROC pglAttachShader;
extern PFNGLBINDBUFFERPROC pglBindBuffer;
extern PFNGLBINDFRAMEBUFFEREXTPROC pglBindFramebufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC pglBindRenderbufferEXT;
extern PFNGLBUFFERDATAPROC pglBufferData;
extern PFNGLCOMPILESHADERPROC pglCompileShader;
extern PFNGLCREATEPROGRAMPROC pglCreateProgram;
extern PFNGLCREATESHADERPROC pglCreateShader;
extern PFNGLDELETEBUFFERSPROC pglDeleteBuffers;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC pglFramebufferRenderbufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC pglFramebufferTexture2DEXT;
extern PFNGLGENBUFFERSPROC pglGenBuffers;
extern PFNGLGENFRAMEBUFFERSEXTPROC pglGenFramebuffersEXT;
extern PFNGLGETATTRIBLOCATIONPROC pglGetAttribLocation;
extern PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC pglGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC pglGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation;
extern PFNGLLINKPROGRAMPROC pglLinkProgram;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC pglRenderbufferStorageEXT;
extern PFNGLSHADERSOURCEPROC pglShaderSource;
extern PFNGLTEXIMAGE3DPROC pglTexImage3D;
extern PFNGLTEXSUBIMAGE3DPROC pglTexSubImage3D;
extern PFNGLUNIFORM1FPROC pglUniform1f;
extern PFNGLUNIFORM1IPROC pglUniform1i;
extern PFNGLUNIFORM2FVPROC pglUniform2fv;
extern PFNGLUNIFORM3FPROC pglUniform3f;
extern PFNGLUNIFORM3FVPROC pglUniform3fv;
extern PFNGLUNIFORM4FVPROC pglUniform4fv;
extern PFNGLUNIFORMMATRIX4FVPROC pglUniformMatrix4fv;
extern PFNGLUSEPROGRAMPROC pglUseProgram;
extern PFNGLVERTEXATTRIB1FPROC pglVertexAttrib1f;
#endif
#endif
