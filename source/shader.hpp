#pragma once
#include "g_primitive_funcs.h"
#include "backend.hpp"
#include "matrix.hpp"
#include "vector.hpp"
#include "audio.hpp"
#include "lodepng.h"

#define pack_rgba(r,g,b,a) (uint32_t)(r<<24|g<<16|b<<8|a)
#define pack_rgb(r,g,b) (uint32_t)(r<<24|g<<16|b<<8)
#define unpack_r(col) (uint8_t)((col>>24)&0xff)
#define unpack_g(col) (uint8_t)((col>>16)&0xff)
#define unpack_b(col) (uint8_t)((col >> 8)&0xff)
#define unpack_a(col) (uint8_t)(col&0xff)

// replace this with your favorite Assert() implementation
#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)


static void FatalError(const char* message)
{
	MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
	ExitProcess(0);
}

//throw hashmap in here for uniform lookup
class Shader {
private:
	GLuint program;

	GLuint vao;
	
	GLenum buffer_target;
	GLuint vertex_buffer;
	
	GLuint attrib;
	void* data; //user set pointer
	
	Gore::HashMap<GLint, std::string> uniform_map;
	static int hash(std::string str) {
		size_t total = 0;
		for (size_t i = 0; i < str.size(); i++) {
			total += str[i];
		}
		return total % 30;
	}
public:
	Shader() {
		program = 0;
		vao = 0;
		vertex_buffer = 0;
		attrib = 0;
		uniform_map.setHashFunction(hash);
	}
	//copy constructor
	Shader(Shader& x) {
		this->uniform_map = x.uniform_map;
		this->program = x.program;
		this->vao = x.vao;
		this->vertex_buffer = x.vertex_buffer;
		this->buffer_target = x.buffer_target;
	}
	void bind();
	//need a bunch of these for every type, https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUniform.xhtml
	//int overloads
	bool setuniform(std::string uni, GLint n);
	bool setuniform(std::string uni, GLint x, GLint y);
	bool setuniform(std::string uni, GLint x, GLint y, GLint z);
	bool setuniform(std::string uni, GLint x, GLint y, GLint z, GLint w);
	//unsigned int overloads
	bool setuniform(std::string uni, GLuint n);
	bool setuniform(std::string uni, GLuint x, GLuint y);
	bool setuniform(std::string uni, GLuint x, GLuint y, GLuint z);
	bool setuniform(std::string uni, GLuint x, GLuint y, GLuint z, GLuint w);
	//float overloads
	bool setuniform(std::string uni, GLfloat n);
	bool setuniform(std::string uni, vec2 n);
	bool setuniform(std::string uni, vec3 n);
	bool setuniform(std::string uni, vec4 n);
	//double overloads
	bool setuniform(std::string uni, GLdouble n);
	bool setuniform(std::string uni, GLdouble x, GLdouble y);
	bool setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z);
	bool setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	//array overloads
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLfloat* value);
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLint* value);
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLuint* value);
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLdouble* value);
	//matrix overloads

	void compile(const char* vertex, const char* frag);
	void compile(const std::string vert_path, const std::string frag_path);

	//vao
	void genbuffer(GLenum target, GLsizei size);
	void addvertexattrib(GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizei elementoffset);
	void updatebufferdata(GLsizei size);
	void setbufferdata(void* data, GLsizei size, GLenum use);
};
