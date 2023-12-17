#include "g_engine_2d.h"

//to load new gl functions
void* GetGLFuncAddress(const char* name) {
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0x0 || p == (void*)0x01 || p == (void*)0x02 || p == (void*)0x03 || p == (void*)-0x1) {
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	//std::cout << p << "\n";
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

PFNGLBINDBUFFERBASEPROC glBindBufferBase_g;
//PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_g;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_g;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_g;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_g;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_g;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D_g;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer_g;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers_g;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers_g;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage_g;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer_g;

PFNGLUNIFORM1FPROC glUniform1f_g;
PFNGLUNIFORM2FPROC glUniform2f_g;
PFNGLUNIFORM3FPROC glUniform3f_g;
PFNGLUNIFORM4FPROC glUniform4f_g;

PFNGLUNIFORM2IPROC glUniform2i_g;
PFNGLUNIFORM3IPROC glUniform3i_g;
PFNGLUNIFORM4IPROC glUniform4i_g;

PFNGLUNIFORM1UIPROC glUniform1ui_g;
PFNGLUNIFORM2UIPROC glUniform2ui_g;
PFNGLUNIFORM3UIPROC glUniform3ui_g;
PFNGLUNIFORM4UIPROC glUniform4ui_g;

PFNGLUNIFORM1DPROC glUniform1d_g;
PFNGLUNIFORM2DPROC glUniform2d_g;
PFNGLUNIFORM3DPROC glUniform3d_g;
PFNGLUNIFORM4DPROC glUniform4d_g;

PFNGLUNIFORM1UIVPROC glUniform1uiv_g;
PFNGLUNIFORM2UIVPROC glUniform2uiv_g;
PFNGLUNIFORM3UIVPROC glUniform3uiv_g;
PFNGLUNIFORM4UIVPROC glUniform4uiv_g;

PFNGLUNIFORM1IVPROC glUniform1iv_g;
PFNGLUNIFORM2IVPROC glUniform2iv_g;
PFNGLUNIFORM3IVPROC glUniform3iv_g;
PFNGLUNIFORM4IVPROC glUniform4iv_g;

PFNGLUNIFORM1FVPROC glUniform1fv_g;
PFNGLUNIFORM2FVPROC glUniform2fv_g;
PFNGLUNIFORM3FVPROC glUniform3fv_g;
PFNGLUNIFORM4FVPROC glUniform4fv_g;

PFNGLUNIFORM1DVPROC glUniform1dv_g;
PFNGLUNIFORM2DVPROC glUniform2dv_g;
PFNGLUNIFORM3DVPROC glUniform3dv_g;
PFNGLUNIFORM4DVPROC glUniform4dv_g;

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

	glUniform3f_g = (PFNGLUNIFORM3FPROC)GetGLFuncAddress("glUniform3f");
	glUniform2f_g = (PFNGLUNIFORM2FPROC)GetGLFuncAddress("glUniform2f");
	glUniform1f_g = (PFNGLUNIFORM1FPROC)GetGLFuncAddress("glUniform1f");
	glUniform4f_g = (PFNGLUNIFORM4FPROC)GetGLFuncAddress("glUniform4f");
	
	glUniform1i_g = (PFNGLUNIFORM1IPROC)GetGLFuncAddress("glUniform1i");
	glUniform2i_g = (PFNGLUNIFORM2IPROC)GetGLFuncAddress("glUniform2i");
	glUniform3i_g = (PFNGLUNIFORM3IPROC)GetGLFuncAddress("glUniform3i");
	glUniform4i_g = (PFNGLUNIFORM4IPROC)GetGLFuncAddress("glUniform4i");

	glUniform1ui_g = (PFNGLUNIFORM1UIPROC)GetGLFuncAddress("glUniform1ui");
	glUniform2ui_g = (PFNGLUNIFORM2UIPROC)GetGLFuncAddress("glUniform2ui");
	glUniform3ui_g = (PFNGLUNIFORM3UIPROC)GetGLFuncAddress("glUniform3ui");
	glUniform4ui_g = (PFNGLUNIFORM4UIPROC)GetGLFuncAddress("glUniform4ui");

	glUniform1d_g = (PFNGLUNIFORM1DPROC)GetGLFuncAddress("glUniform1d");
	glUniform2d_g = (PFNGLUNIFORM2DPROC)GetGLFuncAddress("glUniform2d");
	glUniform3d_g = (PFNGLUNIFORM3DPROC)GetGLFuncAddress("glUniform3d");
	glUniform4d_g = (PFNGLUNIFORM4DPROC)GetGLFuncAddress("glUniform4d");

	glUniform1uiv_g = (PFNGLUNIFORM1UIVPROC)GetGLFuncAddress("glUniform1uiv");
	glUniform2uiv_g = (PFNGLUNIFORM2UIVPROC)GetGLFuncAddress("glUniform2uiv");
	glUniform3uiv_g = (PFNGLUNIFORM3UIVPROC)GetGLFuncAddress("glUniform3uiv");
	glUniform4uiv_g = (PFNGLUNIFORM4UIVPROC)GetGLFuncAddress("glUniform4uiv");
	
	glUniform1iv_g = (PFNGLUNIFORM1IVPROC)GetGLFuncAddress("glUniform1iv");
	glUniform2iv_g = (PFNGLUNIFORM2IVPROC)GetGLFuncAddress("glUniform2iv");
	glUniform3iv_g = (PFNGLUNIFORM3IVPROC)GetGLFuncAddress("glUniform3iv");
	glUniform4iv_g = (PFNGLUNIFORM4IVPROC)GetGLFuncAddress("glUniform4iv");

	glUniform1fv_g = (PFNGLUNIFORM1FVPROC)GetGLFuncAddress("glUniform1fv");
	glUniform2fv_g = (PFNGLUNIFORM2FVPROC)GetGLFuncAddress("glUniform2fv");
	glUniform3fv_g = (PFNGLUNIFORM3FVPROC)GetGLFuncAddress("glUniform3fv");
	glUniform4fv_g = (PFNGLUNIFORM4FVPROC)GetGLFuncAddress("glUniform4fv");

	glUniform1dv_g = (PFNGLUNIFORM1DVPROC)GetGLFuncAddress("glUniform1dv_g");
	glUniform2dv_g = (PFNGLUNIFORM2DVPROC)GetGLFuncAddress("glUniform2dv_g");
	glUniform3dv_g = (PFNGLUNIFORM3DVPROC)GetGLFuncAddress("glUniform3dv_g");
	glUniform4dv_g = (PFNGLUNIFORM4DVPROC)GetGLFuncAddress("glUniform4dv_g");

	glBindBufferBase_g = (PFNGLBINDBUFFERBASEPROC)GetGLFuncAddress("glBindBufferBase");
	//glDebugMessageCallback_g = (PFNGLDEBUGMESSAGECALLBACKPROC)GetGLFuncAddress("glDebugMessageCallback");

	
	glGenFramebuffers_g = (PFNGLGENFRAMEBUFFERSPROC)GetGLFuncAddress("glGenFramebuffers");
	glBindFramebuffer_g = (PFNGLBINDFRAMEBUFFERPROC)GetGLFuncAddress("glBindFramebuffer");
	glCheckFramebufferStatus_g = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetGLFuncAddress("glCheckFramebufferStatus");
	glDeleteFramebuffers_g = (PFNGLDELETEFRAMEBUFFERSPROC)GetGLFuncAddress("glDeleteFramebuffers");
	glFramebufferTexture2D_g = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetGLFuncAddress("glFramebufferTexture2D");
	glBindRenderbuffer_g = (PFNGLBINDRENDERBUFFERPROC)GetGLFuncAddress("glBindRenderbuffer");
	glDeleteRenderbuffers_g = (PFNGLDELETERENDERBUFFERSPROC)GetGLFuncAddress("glDeleteRenderbuffers");
	glGenRenderbuffers_g = (PFNGLGENRENDERBUFFERSPROC)GetGLFuncAddress("glGenRenderbuffers");
	glRenderbufferStorage_g = (PFNGLRENDERBUFFERSTORAGEPROC)GetGLFuncAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer_g = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)GetGLFuncAddress("glFramebufferRenderbuffer");



}