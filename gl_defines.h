#pragma once
#define GL_GLEXT_PROTOTYPES
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <gl/GL.h>
//#include <gl/GLU.h>
//#include <glcorearb.h>
#include <wglext.h>
#include <glext.h>
//#define APIENTRYP APIENTRY *


//#pragma comment (lib, "gdi32.lib")
//#pragma comment (lib, "user32.lib")
//#pragma comment (lib, "opengl32.lib")


//typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);

#define NUMFUNCTIONS 13


extern void* myglfunc[NUMFUNCTIONS];

#define glBindBuffer ((PFNGLBINDBUFFERPROC)myglfunc[0])
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

//PFNGLBINDBUFFERPROC glBindBuffer = nullptr;


