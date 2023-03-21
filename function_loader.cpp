#include "g_engine_2d.h"

//to load new gl functions
void* GetGLFuncAddress(const char* name) {
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0x0 || p == (void*)0x01 || p == (void*)0x02 || p == (void*)0x03 || p == (void*)-0x1) {
		HMODULE module = GetModuleHandleA("opengl32.lib");
		p = (void*)GetProcAddress(module, name);
	}
	return p;
}


static const char* strs[] = {
	"glBindBuffer", "glGenBuffers", "glAttachShader", "glCompileShader", "glCreateProgram", "glCreateShader",
	"glDeleteShader", "glGetProgramiv", "glCreateShader", "glGetProgramInfoLog", "glGetShaderiv", "glGetShaderInfoLog",
	"glLinkProgram", "glShaderSource"
};

void* myglfunc[NUMFUNCTIONS];

void EngineNewGL::loadFunctions() {
	
	for (int i = 0; i < NUMFUNCTIONS; i++) {
		myglfunc[i] = GetGLFuncAddress(strs[i]);
	}
}