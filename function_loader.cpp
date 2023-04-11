#include "g_engine_2d.h"

//to load new gl functions
void* GetGLFuncAddress(const char* name) {
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0x0 || p == (void*)0x01 || p == (void*)0x02 || p == (void*)0x03 || p == (void*)-0x1) {
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	std::cout << p << "\n";
	return p;
}





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
PFNGLUNIFORM1IPROC glUniform1i_g;
PFNGLACTIVETEXTUREPROC glActiveTexture_g;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_g;
PFNGLISSHADERPROC glIsShader_g;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_g;
PFNGLVALIDATEPROGRAMPROC glValidateProgram_g;
PFNGLDETACHSHADERPROC glDetachShader_g;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap_g;

PFNGLCREATETEXTURESPROC glCreateTextures_g;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit_g;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri_g;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D_g;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D_g;

PFNGLDRAWARRAYSEXTPROC glDrawArrays_g;
//PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_g;


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
	glUniform1i_g = (PFNGLUNIFORM1IPROC)GetGLFuncAddress("glUniform1i");
	glActiveTexture_g = (PFNGLACTIVETEXTUREPROC)GetGLFuncAddress("glActiveTexture");
	glGetUniformLocation_g = (PFNGLGETUNIFORMLOCATIONPROC)GetGLFuncAddress("glGetUniformLocation");
	glIsShader_g = (PFNGLISSHADERPROC)GetGLFuncAddress("glIsShader");
	glGetAttribLocation_g = (PFNGLGETATTRIBLOCATIONPROC)GetGLFuncAddress("glGetAttribLocation");
	glValidateProgram_g = (PFNGLVALIDATEPROGRAMPROC)GetGLFuncAddress("glValidateProgram");
	glDetachShader_g = (PFNGLDETACHSHADERPROC)GetGLFuncAddress("glDetachShader");
	glGenerateMipmap_g = (PFNGLGENERATEMIPMAPPROC)GetGLFuncAddress("glGenerateMipmap");

	glCreateTextures_g = (PFNGLCREATETEXTURESPROC)GetGLFuncAddress("glCreateTextures");
	glBindTextureUnit_g = (PFNGLBINDTEXTUREUNITPROC)GetGLFuncAddress("glBindTextureUnit");
	glTextureParameteri_g = (PFNGLTEXTUREPARAMETERIPROC)GetGLFuncAddress("glTextureParameteri");
	glTextureStorage2D_g = (PFNGLTEXTURESTORAGE2DPROC)GetGLFuncAddress("glTextureStorage2D");
	glTextureSubImage2D_g = (PFNGLTEXTURESUBIMAGE2DPROC)GetGLFuncAddress("glTextureSubImage2D");

	glDrawArrays_g = (PFNGLDRAWARRAYSEXTPROC)GetGLFuncAddress("glDrawArrays");
	//glDebugMessageCallback_g = (PFNGLDEBUGMESSAGECALLBACKPROC)GetGLFuncAddress("glDebugMessageCallback");

}