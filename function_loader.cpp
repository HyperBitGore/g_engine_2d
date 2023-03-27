#include "g_engine_2d.h"

//to load new gl functions
void* GetGLFuncAddress(const char* name) {
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0x0 || p == (void*)0x01 || p == (void*)0x02 || p == (void*)0x03 || p == (void*)-0x1) {
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	return p;
}


static const char* strs[] = {
	"glBindBuffer", "glGenBuffers", "glAttachShader", "glCompileShader", "glCreateProgram", "glCreateShader",
	"glDeleteShader", "glGetProgramiv", "glCreateShader", "glGetProgramInfoLog", "glGetShaderiv", "glGetShaderInfoLog",
	"glLinkProgram", "glShaderSource", "glVertexAttribPointer", "glEnableVertexAttribArray", "glUseProgram", "glBufferData",
	"glDisableVertexAttribArray", "glGenVertexArrays", "glBindVertexArray"
};

void* myglfunc[NUMFUNCTIONS];


PFNGLBINDBUFFERPROC glBindBuffer_g;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_g;
PFNGLGENBUFFERSPROC glGenBuffers_g;
PFNGLBUFFERDATAPROC glBufferData_g;
PFNGLATTACHSHADERPROC glAttachShader_g;
PFNGLCOMPILESHADERPROC glCompileShader_g;
PFNGLCREATEPROGRAMPROC glCreateProgram_g;
PFNGLCREATESHADERPROC glCreateShader_g;
PFNGLDELETESHADERPROC glDeleteShader_g;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_g;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_g;
PFNGLGETPROGRAMIVPROC glGetProgramiv_g;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_g;
PFNGLLINKPROGRAMPROC glLinkProgram_g;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_g;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray_g;
PFNGLSHADERSOURCEPROC glShaderSource_g;
PFNGLGETSHADERIVPROC glGetShaderiv_g;
PFNGLUSEPROGRAMPROC glUseProgram_g;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_g;

void EngineNewGL::loadFunctions() {
	glBindBuffer_g = (PFNGLBINDBUFFERPROC)GetGLFuncAddress("glBindBuffer");
	glGenVertexArrays_g = (PFNGLGENVERTEXARRAYSPROC)GetGLFuncAddress("glGenVertexArrays");
	glGenBuffers_g = (PFNGLGENBUFFERSPROC)GetGLFuncAddress("glGenBuffers");
	glBufferData_g = (PFNGLBUFFERDATAPROC)GetGLFuncAddress("glBufferData");
	glAttachShader_g = (PFNGLATTACHSHADERPROC)GetGLFuncAddress("glAttachShader");
	glCompileShader_g = (PFNGLCOMPILESHADERPROC)GetGLFuncAddress("glCompileShader");
	glCreateProgram_g = (PFNGLCREATEPROGRAMPROC)GetGLFuncAddress("glCreateProgram");
	glCreateShader_g = (PFNGLCREATESHADERPROC)GetGLFuncAddress("glCreateShader");
	glDeleteShader_g = (PFNGLDELETESHADERPROC)GetGLFuncAddress("glDeleteShader");
	glDisableVertexAttribArray_g = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)GetGLFuncAddress("glDisableVertexAttribArray");
	glEnableVertexAttribArray_g = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetGLFuncAddress("glEnableVertexAttribArray");
	glGetProgramiv_g = (PFNGLGETPROGRAMIVPROC)GetGLFuncAddress("glGetProgramiv");
	glGetProgramInfoLog_g = (PFNGLGETPROGRAMINFOLOGPROC)GetGLFuncAddress("glGetProgramInfoLog");
	glLinkProgram_g = (PFNGLLINKPROGRAMPROC)GetGLFuncAddress("glLinkProgram");
	glGetShaderInfoLog_g = (PFNGLGETSHADERINFOLOGPROC)GetGLFuncAddress("glGetShaderInfoLog");
	glBindVertexArray_g = (PFNGLBINDVERTEXARRAYPROC)GetGLFuncAddress("glBindVertexArray");
	glShaderSource_g = (PFNGLSHADERSOURCEPROC)GetGLFuncAddress("glShaderSource");
	glGetShaderiv_g = (PFNGLGETSHADERIVPROC)GetGLFuncAddress("glGetShaderiv");
	glUseProgram_g = (PFNGLUSEPROGRAMPROC)GetGLFuncAddress("glUseProgram");
	glVertexAttribPointer_g = (PFNGLVERTEXATTRIBPOINTERPROC)GetGLFuncAddress("glVertexAttribPointer");

	for (int i = 0; i < NUMFUNCTIONS; i++) {
		myglfunc[i] = GetGLFuncAddress(strs[i]);
	}
}