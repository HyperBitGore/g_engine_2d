#pragma once
#define GL_GLEXT_PROTOTYPES
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <Windows.h>
//#include <GL/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
//#include <glcorearb.h>
#include <wglext.h>
#include <glext.h>


//#pragma comment (lib, "gdi32.lib")
//#pragma comment (lib, "user32.lib")
//#pragma comment (lib, "opengl32.lib")


//typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);

#define NUMFUNCTIONS 20


extern void* myglfunc[NUMFUNCTIONS];


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


/*#define glBindBuffer ((PFNGLBINDBUFFERPROC)myglfunc[0])
#define glGenBuffers ((PFNGLGENBUFFERSPROC)myglfunc[1])
#define glAttachShader ((PFNGLATTACHSHADERPROC)myglfunc[2])
#define glCompileShader ((PFNGLCOMPILESHADERPROC)myglfunc[3])
#define glCreateProgram ((PFNGLCREATEPROGRAMPROC)myglfunc[4])
#define glDeleteShader ((PFNGLDELETESHADERPROC)myglfunc[5])
#define glGetProgramiv ((PFNGLGETPROGRAMIVPROC)myglfunc[6])
#define glCreateShader ((PFNGLCREATESHADERPROC)myglfunc[7])
#define glGetProgramInfoLog ((PFNGLGETPROGRAMINFOLOGPROC)myglfunc[8])
#define glGetShaderiv ((PFNGLGETSHADERIVPROC)myglfunc[9])
#define glGetShaderInfoLog ((PFNGLGETSHADERINFOLOGPROC)myglfunc[10])
#define glLinkProgram ((PFNGLLINKPROGRAMPROC)myglfunc[11])
#define glShaderSource ((PFNGLSHADERSOURCEPROC)myglfunc[12])
#define glVertexAttribPointer ((PFNGLVERTEXATTRIBPOINTERPROC)myglfunc[13])
#define glEnableVertexAttribArray ((PFNGLENABLEVERTEXATTRIBARRAYPROC)myglfunc[14])
#define glUseProgram ((PFNGLUSEPROGRAMPROC)myglfunc[15])
#define glBufferData ((PFNGLBUFFERDATAPROC)myglfunc[16])
#define glDisableVertexAttribArray ((PFNGLDISABLEVERTEXATTRIBARRAYPROC)myglfunc[17])
#define glGenVertexArrays ((PFNGLGENVERTEXARRAYSPROC)myglfunc[18])
#define glBindVertexArray ((PFNGLBINDVERTEXARRAYPROC)myglfunc[19])*/

//#define glDrawArrays (()myglfunc[17])


//PFNGLBINDBUFFERPROC glBindBuffer = nullptr;

//try this method and if doesnt work switch to using macros again

//typedef void WINAPI glUseProgram_g(GLuint program);
//typedef void WINAPI glGenVertexArrays_g(GLsizei n, GLuint* arrays);


//extern glUseProgram_g* glUseProgram = nullptr;
//extern glGenVertexArrays_g* glGenVertexArrays = nullptr;