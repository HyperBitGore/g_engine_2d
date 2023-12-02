#pragma once
#define GL_GLEXT_PROTOTYPES
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

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
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap_g;

extern PFNGLCREATETEXTURESPROC glCreateTextures_g;
//extern PFNGLBINDTEXTUREUNITPROC glBindTextureUnit_g;
extern PFNGLTEXTUREPARAMETERIPROC glTextureParameteri_g;
extern PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D_g;
extern PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D_g;
extern PFNGLDRAWARRAYSEXTPROC glDrawArrays_g;

extern PFNGLUNIFORM2FPROC glUniform2f_g;
extern PFNGLUNIFORM1FPROC glUniform1f_g;
extern PFNGLUNIFORM4FPROC glUniform4f_g;

extern PFNGLUNIFORM1UIVPROC glUniform1uiv_g;
extern PFNGLUNIFORM1IVPROC glUniform1iv_g;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase_g;
//extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_g;

extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_g;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_g;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_g;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_g;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D_g;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer_g;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers_g;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers_g;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage_g;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer_g;