#pragma once
#define GL_GLEXT_PROTOTYPES
//#define WIN32_LEAN_AND_MEAN
//#define WIN32_EXTRA_LEAN

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
//#include <glcorearb.h>
#include <wglext.h>
#include <glext.h>


//#pragma comment (lib, "gdi32.lib")
//#pragma comment (lib, "user32.lib")
//#pragma comment (lib, "opengl32.lib")





extern PFNGLBINDBUFFERPROC glBindBuffer_g;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_g;
extern PFNGLGENBUFFERSPROC glGenBuffers_g;
extern PFNGLBUFFERDATAPROC glBufferData_g;
extern PFNGLATTACHSHADERPROC glAttachShader_g;
extern PFNGLCOMPILESHADERPROC glCompileShader_g;
extern PFNGLCREATEPROGRAMPROC glCreateProgram_g;
extern PFNGLCREATESHADERPROC glCreateShader_g;
extern PFNGLDELETESHADERPROC glDeleteShader_g;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_g;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_g;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv_g;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_g;
extern PFNGLLINKPROGRAMPROC glLinkProgram_g;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_g;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray_g;
extern PFNGLSHADERSOURCEPROC glShaderSource_g;
extern PFNGLGETSHADERIVPROC glGetShaderiv_g;
extern PFNGLUSEPROGRAMPROC glUseProgram_g;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_g;
extern PFNGLUNIFORM1IPROC glUniform1i_g;
extern PFNGLACTIVETEXTUREPROC glActiveTexture_g;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_g;
extern PFNGLISSHADERPROC glIsShader_g;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_g;
extern PFNGLVALIDATEPROGRAMPROC glValidateProgram_g;
extern PFNGLDETACHSHADERPROC glDetachShader_g;