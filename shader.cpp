#include "g_engine_2d.h"


void Shader::bind() {
	glUseProgram_g(program);
}

bool Shader::setuniform(std::string uni, GLint n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform1i_g(point, n);
			uniform_map.insert(uni, n);
			return true;
		}
		return false;
	}
	glUniform1i_g(*uf, n);
	return true;
}


void Shader::compile(const char* vertex, const char* frag) {
	
}
void Shader::compile(const std::string vert_path, const std::string frag_path) {
	
}