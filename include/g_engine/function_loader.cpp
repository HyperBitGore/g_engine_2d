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


float toRadians (float degrees) {
	return degrees * M_PI / 180.0f;
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
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLISSHADERPROC glIsShader;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLDELETEPROGRAMPROC glDeleteProgram;

PFNGLMAPBUFFERPROC glMapBuffer;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange;

PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;

PFNGLDRAWARRAYSEXTPROC glDrawArraysExt;

PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLGETSTRINGIPROC glGetStringi;
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

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

PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv;
PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv;
PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv;

PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

#if defined(_WIN32)
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif

bool gore::loadGLFunctions(std::function<void*(const char*)> getProc) {
	static bool loaded = false;
	if (loaded) return true;
	std::function<void*(const char*)> loadFunc = GetGLFuncAddress;
	if (getProc != nullptr) {
		loadFunc = getProc;
	}

	glBindBuffer = (PFNGLBINDBUFFERPROC)loadFunc("glBindBuffer");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)loadFunc("glGenVertexArrays");
	glGenBuffers = (PFNGLGENBUFFERSPROC)loadFunc("glGenBuffers");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)loadFunc("glDeleteBuffers");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)loadFunc("glDeleteVertexArrays");
	glBufferData = (PFNGLBUFFERDATAPROC)loadFunc("glBufferData");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)loadFunc("glBufferSubData");
	glMapBuffer = (PFNGLMAPBUFFERPROC)loadFunc("glMapBuffer");
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)loadFunc("glUnmapBuffer");
	glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)loadFunc("glMapBufferRange");

	glAttachShader = (PFNGLATTACHSHADERPROC)loadFunc("glAttachShader");
	glCompileShader = (PFNGLCOMPILESHADERPROC)loadFunc("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)loadFunc("glCreateProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)loadFunc("glDeleteProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)loadFunc("glCreateShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)loadFunc("glDeleteShader");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)loadFunc("glDisableVertexAttribArray");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)loadFunc("glEnableVertexAttribArray");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)loadFunc("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)loadFunc("glGetProgramInfoLog");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)loadFunc("glLinkProgram");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)loadFunc("glGetShaderInfoLog");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)loadFunc("glBindVertexArray");
	glShaderSource = (PFNGLSHADERSOURCEPROC)loadFunc("glShaderSource");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)loadFunc("glGetShaderiv");
	glUseProgram = (PFNGLUSEPROGRAMPROC)loadFunc("glUseProgram");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)loadFunc("glVertexAttribPointer");
	glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)loadFunc("glVertexAttribIPointer");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)loadFunc("glGetUniformLocation");
	glIsShader = (PFNGLISSHADERPROC)loadFunc("glIsShader");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)loadFunc("glGetAttribLocation");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)loadFunc("glValidateProgram");
	glDetachShader = (PFNGLDETACHSHADERPROC)loadFunc("glDetachShader");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)loadFunc("glGenerateMipmap");

	glCreateTextures = (PFNGLCREATETEXTURESPROC)loadFunc("glCreateTextures");
	glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)loadFunc("glBindTextureUnit");
	glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)loadFunc("glTextureParameteri");
	glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)loadFunc("glTextureStorage2D");
	glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)loadFunc("glTextureSubImage2D");
	glDrawArraysExt = (PFNGLDRAWARRAYSEXTPROC)loadFunc("glDrawArrays");

	glUniform3f = (PFNGLUNIFORM3FPROC)loadFunc("glUniform3f");
	glUniform2f = (PFNGLUNIFORM2FPROC)loadFunc("glUniform2f");
	glUniform1f = (PFNGLUNIFORM1FPROC)loadFunc("glUniform1f");
	glUniform4f = (PFNGLUNIFORM4FPROC)loadFunc("glUniform4f");
	
	glUniform1i = (PFNGLUNIFORM1IPROC)loadFunc("glUniform1i");
	glUniform2i = (PFNGLUNIFORM2IPROC)loadFunc("glUniform2i");
	glUniform3i = (PFNGLUNIFORM3IPROC)loadFunc("glUniform3i");
	glUniform4i = (PFNGLUNIFORM4IPROC)loadFunc("glUniform4i");

	glUniform1ui = (PFNGLUNIFORM1UIPROC)loadFunc("glUniform1ui");
	glUniform2ui = (PFNGLUNIFORM2UIPROC)loadFunc("glUniform2ui");
	glUniform3ui = (PFNGLUNIFORM3UIPROC)loadFunc("glUniform3ui");
	glUniform4ui = (PFNGLUNIFORM4UIPROC)loadFunc("glUniform4ui");

	glUniform1d = (PFNGLUNIFORM1DPROC)loadFunc("glUniform1d");
	glUniform2d = (PFNGLUNIFORM2DPROC)loadFunc("glUniform2d");
	glUniform3d = (PFNGLUNIFORM3DPROC)loadFunc("glUniform3d");
	glUniform4d = (PFNGLUNIFORM4DPROC)loadFunc("glUniform4d");

	glUniform1uiv = (PFNGLUNIFORM1UIVPROC)loadFunc("glUniform1uiv");
	glUniform2uiv = (PFNGLUNIFORM2UIVPROC)loadFunc("glUniform2uiv");
	glUniform3uiv = (PFNGLUNIFORM3UIVPROC)loadFunc("glUniform3uiv");
	glUniform4uiv = (PFNGLUNIFORM4UIVPROC)loadFunc("glUniform4uiv");
	
	glUniform1iv = (PFNGLUNIFORM1IVPROC)loadFunc("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)loadFunc("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)loadFunc("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)loadFunc("glUniform4iv");

	glUniform1fv = (PFNGLUNIFORM1FVPROC)loadFunc("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)loadFunc("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)loadFunc("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)loadFunc("glUniform4fv");

	glUniform1dv = (PFNGLUNIFORM1DVPROC)loadFunc("glUniform1dv");
	glUniform2dv = (PFNGLUNIFORM2DVPROC)loadFunc("glUniform2dv");
	glUniform3dv = (PFNGLUNIFORM3DVPROC)loadFunc("glUniform3dv");
	glUniform4dv = (PFNGLUNIFORM4DVPROC)loadFunc("glUniform4dv");

	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)loadFunc("glBindBufferBase");
	glGetStringi = (PFNGLGETSTRINGIPROC)loadFunc("glGetStringi");
	glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)loadFunc("glDebugMessageCallback");

	glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)loadFunc("glVertexAttribDivisor");
	glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)loadFunc("glDrawArraysInstanced");
	glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)loadFunc("glDrawElementsInstanced");

	
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)loadFunc("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)loadFunc("glBindFramebuffer");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)loadFunc("glCheckFramebufferStatus");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)loadFunc("glDeleteFramebuffers");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)loadFunc("glFramebufferTexture2D");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)loadFunc("glBindRenderbuffer");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)loadFunc("glDeleteRenderbuffers");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)loadFunc("glGenRenderbuffers");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)loadFunc("glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)loadFunc("glFramebufferRenderbuffer");

	glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)loadFunc("glUniformMatrix2fv");
	glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)loadFunc("glUniformMatrix3fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)loadFunc("glUniformMatrix4fv");

	glUniformMatrix2dv = (PFNGLUNIFORMMATRIX2DVPROC)loadFunc("glUniformMatrix2dv");
	glUniformMatrix3dv = (PFNGLUNIFORMMATRIX3DVPROC)loadFunc("glUniformMatrix3dv");
	glUniformMatrix4dv = (PFNGLUNIFORMMATRIX4DVPROC)loadFunc("glUniformMatrix4dv");

	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)loadFunc("glBlitFramebuffer");

	#if defined(_WIN32)
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)loadFunc("glActiveTexture");
	#endif
	loaded = true;
	return true;
}