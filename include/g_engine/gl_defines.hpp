#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>

#include <gl/GL.h>
#include <gl/GLU.h>
#include "../GL/wglext.h"
#include "../GL/glext.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <corecrt_math_defines.h>
#endif

#if defined(__unix__)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>  // For Linux (X11)
#endif

typedef void (APIENTRYP PFNGLDRAWELEMENTSEXTPROC)
    (GLenum mode, GLsizei count, GLenum type, const void* indices);

extern float toRadians (float degrees);

extern void* GetGLFuncAddress(const char* name);

extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLISSHADERPROC glIsShader;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLVALIDATEPROGRAMPROC glValidateProgram;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;

extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRange;

extern PFNGLCREATETEXTURESPROC glCreateTextures;
extern PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;
extern PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
extern PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
extern PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
extern PFNGLDRAWARRAYSEXTPROC glDrawArraysExt;
extern PFNGLDRAWELEMENTSEXTPROC glDrawElementsExt;


extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;

extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;

extern PFNGLUNIFORM1DPROC glUniform1d;
extern PFNGLUNIFORM2DPROC glUniform2d;
extern PFNGLUNIFORM3DPROC glUniform3d;
extern PFNGLUNIFORM4DPROC glUniform4d;

extern PFNGLUNIFORM2IPROC glUniform2i;
extern PFNGLUNIFORM3IPROC glUniform3i;
extern PFNGLUNIFORM4IPROC glUniform4i;

extern PFNGLUNIFORM1UIPROC glUniform1ui;
extern PFNGLUNIFORM2UIPROC glUniform2ui;
extern PFNGLUNIFORM3UIPROC glUniform3ui;
extern PFNGLUNIFORM4UIPROC glUniform4ui;

extern PFNGLUNIFORM1IVPROC glUniform1iv;
extern PFNGLUNIFORM2IVPROC glUniform2iv;
extern PFNGLUNIFORM3IVPROC glUniform3iv;
extern PFNGLUNIFORM4IVPROC glUniform4iv;

extern PFNGLUNIFORM1UIVPROC glUniform1uiv;
extern PFNGLUNIFORM2UIVPROC glUniform2uiv;
extern PFNGLUNIFORM3UIVPROC glUniform3uiv;
extern PFNGLUNIFORM4UIVPROC glUniform4uiv;

extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;

extern PFNGLUNIFORM1DVPROC glUniform1dv;
extern PFNGLUNIFORM2DVPROC glUniform2dv;
extern PFNGLUNIFORM3DVPROC glUniform3dv;
extern PFNGLUNIFORM4DVPROC glUniform4dv;

extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

extern PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv;
extern PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv;
extern PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv;

extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

extern PFNGLGETSTRINGIPROC glGetStringi;
extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
extern PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;

extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
extern PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirect;
extern PFNGLGETTEXTUREHANDLEARBPROC glGetTextureHandleARB;
extern PFNGLMAKETEXTUREHANDLERESIDENTARBPROC glMakeTextureHandleResidentARB;
extern PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC glMakeTextureHandleNonResidentARB;
extern PFNGLISTEXTUREHANDLERESIDENTARBPROC glIsTextureHandleResidentARB;
extern PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer;

#if defined(_WIN32)
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
namespace gore {
    bool loadGLFunctions (std::function<void*(const char*)> getProc = nullptr);
}

#if defined(_UNIT_TEST_)
namespace gore {
class gl_logger {
public:
    std::vector<uint8_t> buffer_data;
    std::vector<std::vector<uint8_t>> calls;
    std::string function_name;
    // rule of 5
    gl_logger (std::string function_name) {
        this->function_name = function_name;
    }
    template <typename... Args>
    void logCall(const Args&... args) {
        static_assert((std::is_trivially_copyable_v<Args> && ...));
        std::vector<uint8_t> call_data;
        (append(call_data, args), ...);
        buffer_data = call_data;
        calls.push_back(std::move(call_data));
    }
    template <typename... Args>
    bool hasCallWithPrefix(const Args&... args) const {
        static_assert((std::is_trivially_copyable_v<Args> && ...));
        std::vector<uint8_t> expected;
        (append(expected, args), ...);
        for (const auto& call : calls) {
            if (call.size() >= expected.size() &&
                std::equal(expected.begin(), expected.end(), call.begin())) {
                return true;
            }
        }
        return false;
    }
    gl_logger (const gl_logger& gl) {
        this->buffer_data = gl.buffer_data;
        this->calls = gl.calls;
        this->function_name = gl.function_name;
    }
    gl_logger (gl_logger&& gl) {
        this->buffer_data = std::move(gl.buffer_data);
        this->calls = std::move(gl.calls);
        this->function_name = std::move(gl.function_name);
    }
    gl_logger& operator=(const gl_logger& gl) {
        if (this != &gl) {
            this->buffer_data = gl.buffer_data;
            this->calls = gl.calls;
            this->function_name = gl.function_name;
        }
        return *this;
    }
    gl_logger& operator=(gl_logger&& gl) {
        if (this != &gl) {
            this->buffer_data = std::move(gl.buffer_data);
            this->calls = std::move(gl.calls);
            this->function_name = std::move(gl.function_name);
        }
        return *this;
    }

private:
    template <typename T>
    static void append(std::vector<uint8_t>& destination, const T& value) {
        const auto* bytes = reinterpret_cast<const uint8_t*>(&value);
        destination.insert(destination.end(), bytes, bytes + sizeof(T));
    }
};

extern gl_logger draw_arrays_log;
extern gl_logger bind_buffer_log;
extern gl_logger buffer_data_log;
extern gl_logger buffer_sub_data_log;
extern gl_logger bind_vertex_array_log;
extern gl_logger enable_vertex_attrib_array_log;
extern gl_logger vertex_attrib_log;
extern gl_logger vertex_attrib_i_log;
extern gl_logger bind_buffer_base_log;
extern gl_logger draw_elements_log;
}
#endif