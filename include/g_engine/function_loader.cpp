#include "g_engine_2d.hpp"

//to load new gl functions
void* GetGLFuncAddress(const char* name) {
	#if defined(__unix__)
	void* p = (void*)glXGetProcAddress((GLubyte*)name);
	#endif
	#if defined(_WIN32)
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0x0 || p == (void*)0x01 || p == (void*)0x02 || p == (void*)0x03 || p == (void*)-0x1) {
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(module, name);
	}
	#endif
	return p;
}





PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;

PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLISSHADERPROC glIsShader;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;

PFNGLDRAWARRAYSEXTPROC glDrawArraysExt;

PFNGLBINDBUFFERBASEPROC glBindBufferBase;
//PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;

PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4IPROC glUniform4i;

PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2UIPROC glUniform2ui;
PFNGLUNIFORM3UIPROC glUniform3ui;
PFNGLUNIFORM4UIPROC glUniform4ui;

PFNGLUNIFORM1DPROC glUniform1d;
PFNGLUNIFORM2DPROC glUniform2d;
PFNGLUNIFORM3DPROC glUniform3d;
PFNGLUNIFORM4DPROC glUniform4d;

PFNGLUNIFORM1UIVPROC glUniform1uiv;
PFNGLUNIFORM2UIVPROC glUniform2uiv;
PFNGLUNIFORM3UIVPROC glUniform3uiv;
PFNGLUNIFORM4UIVPROC glUniform4uiv;

PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4IVPROC glUniform4iv;

PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;

PFNGLUNIFORM1DVPROC glUniform1dv;
PFNGLUNIFORM2DVPROC glUniform2dv;
PFNGLUNIFORM3DVPROC glUniform3dv;
PFNGLUNIFORM4DVPROC glUniform4dv;

PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

#if defined(_WIN32)
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif

void gore::g_engine_2d::loadFunctions() {
	glBindBuffer = (PFNGLBINDBUFFERPROC)GetGLFuncAddress("glBindBuffer");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GetGLFuncAddress("glGenVertexArrays");
	glGenBuffers = (PFNGLGENBUFFERSPROC)GetGLFuncAddress("glGenBuffers");
	glBufferData = (PFNGLBUFFERDATAPROC)GetGLFuncAddress("glBufferData");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)GetGLFuncAddress("glBufferSubData");

	glAttachShader = (PFNGLATTACHSHADERPROC)GetGLFuncAddress("glAttachShader");
	glCompileShader = (PFNGLCOMPILESHADERPROC)GetGLFuncAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetGLFuncAddress("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)GetGLFuncAddress("glCreateShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)GetGLFuncAddress("glDeleteShader");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)GetGLFuncAddress("glDisableVertexAttribArray");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetGLFuncAddress("glEnableVertexAttribArray");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GetGLFuncAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GetGLFuncAddress("glGetProgramInfoLog");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)GetGLFuncAddress("glLinkProgram");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GetGLFuncAddress("glGetShaderInfoLog");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetGLFuncAddress("glBindVertexArray");
	glShaderSource = (PFNGLSHADERSOURCEPROC)GetGLFuncAddress("glShaderSource");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)GetGLFuncAddress("glGetShaderiv");
	glUseProgram = (PFNGLUSEPROGRAMPROC)GetGLFuncAddress("glUseProgram");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GetGLFuncAddress("glVertexAttribPointer");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GetGLFuncAddress("glGetUniformLocation");
	glIsShader = (PFNGLISSHADERPROC)GetGLFuncAddress("glIsShader");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)GetGLFuncAddress("glGetAttribLocation");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)GetGLFuncAddress("glValidateProgram");
	glDetachShader = (PFNGLDETACHSHADERPROC)GetGLFuncAddress("glDetachShader");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)GetGLFuncAddress("glGenerateMipmap");

	glCreateTextures = (PFNGLCREATETEXTURESPROC)GetGLFuncAddress("glCreateTextures");
	glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)GetGLFuncAddress("glBindTextureUnit");
	glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)GetGLFuncAddress("glTextureParameteri");
	glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)GetGLFuncAddress("glTextureStorage2D");
	glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)GetGLFuncAddress("glTextureSubImage2D");
	glDrawArraysExt = (PFNGLDRAWARRAYSEXTPROC)GetGLFuncAddress("glDrawArrays");

	glUniform3f = (PFNGLUNIFORM3FPROC)GetGLFuncAddress("glUniform3f");
	glUniform2f = (PFNGLUNIFORM2FPROC)GetGLFuncAddress("glUniform2f");
	glUniform1f = (PFNGLUNIFORM1FPROC)GetGLFuncAddress("glUniform1f");
	glUniform4f = (PFNGLUNIFORM4FPROC)GetGLFuncAddress("glUniform4f");
	
	glUniform1i = (PFNGLUNIFORM1IPROC)GetGLFuncAddress("glUniform1i");
	glUniform2i = (PFNGLUNIFORM2IPROC)GetGLFuncAddress("glUniform2i");
	glUniform3i = (PFNGLUNIFORM3IPROC)GetGLFuncAddress("glUniform3i");
	glUniform4i = (PFNGLUNIFORM4IPROC)GetGLFuncAddress("glUniform4i");

	glUniform1ui = (PFNGLUNIFORM1UIPROC)GetGLFuncAddress("glUniform1ui");
	glUniform2ui = (PFNGLUNIFORM2UIPROC)GetGLFuncAddress("glUniform2ui");
	glUniform3ui = (PFNGLUNIFORM3UIPROC)GetGLFuncAddress("glUniform3ui");
	glUniform4ui = (PFNGLUNIFORM4UIPROC)GetGLFuncAddress("glUniform4ui");

	glUniform1d = (PFNGLUNIFORM1DPROC)GetGLFuncAddress("glUniform1d");
	glUniform2d = (PFNGLUNIFORM2DPROC)GetGLFuncAddress("glUniform2d");
	glUniform3d = (PFNGLUNIFORM3DPROC)GetGLFuncAddress("glUniform3d");
	glUniform4d = (PFNGLUNIFORM4DPROC)GetGLFuncAddress("glUniform4d");

	glUniform1uiv = (PFNGLUNIFORM1UIVPROC)GetGLFuncAddress("glUniform1uiv");
	glUniform2uiv = (PFNGLUNIFORM2UIVPROC)GetGLFuncAddress("glUniform2uiv");
	glUniform3uiv = (PFNGLUNIFORM3UIVPROC)GetGLFuncAddress("glUniform3uiv");
	glUniform4uiv = (PFNGLUNIFORM4UIVPROC)GetGLFuncAddress("glUniform4uiv");
	
	glUniform1iv = (PFNGLUNIFORM1IVPROC)GetGLFuncAddress("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)GetGLFuncAddress("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)GetGLFuncAddress("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)GetGLFuncAddress("glUniform4iv");

	glUniform1fv = (PFNGLUNIFORM1FVPROC)GetGLFuncAddress("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)GetGLFuncAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)GetGLFuncAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)GetGLFuncAddress("glUniform4fv");

	glUniform1dv = (PFNGLUNIFORM1DVPROC)GetGLFuncAddress("glUniform1dv");
	glUniform2dv = (PFNGLUNIFORM2DVPROC)GetGLFuncAddress("glUniform2dv");
	glUniform3dv = (PFNGLUNIFORM3DVPROC)GetGLFuncAddress("glUniform3dv");
	glUniform4dv = (PFNGLUNIFORM4DVPROC)GetGLFuncAddress("glUniform4dv");

	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)GetGLFuncAddress("glBindBufferBase");
	//glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)GetGLFuncAddress("glDebugMessageCallback");

	
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GetGLFuncAddress("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GetGLFuncAddress("glBindFramebuffer");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetGLFuncAddress("glCheckFramebufferStatus");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)GetGLFuncAddress("glDeleteFramebuffers");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetGLFuncAddress("glFramebufferTexture2D");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)GetGLFuncAddress("glBindRenderbuffer");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)GetGLFuncAddress("glDeleteRenderbuffers");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)GetGLFuncAddress("glGenRenderbuffers");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)GetGLFuncAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)GetGLFuncAddress("glFramebufferRenderbuffer");

	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)GetGLFuncAddress("glUniformMatrix4fv");

	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)GetGLFuncAddress("glBlitFramebuffer");

	#if defined(_WIN32)
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)GetGLFuncAddress("glActiveTexture");
	#endif
}