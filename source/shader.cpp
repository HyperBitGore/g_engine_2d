#include "shader.hpp"


void Shader::bind() {
	glUseProgram_g(program);
}

bool Shader::setuniform(std::string uni, GLint n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform1i_g(point, n);
			uniform_map.insert(uni, point);
			return true;
		}
		return false;
	}
	glUniform1i_g(*uf, n);
	return true;
}

bool Shader::setuniform(std::string uni, GLint x, GLint y) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform2i_g(point, x, y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2i_g(*uf, x, y);
	return true;
}
bool Shader::setuniform(std::string uni, GLint x, GLint y, GLint z) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform3i_g(point, x, y, z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3i_g(*uf, x, y, z);
	return true;
}
bool Shader::setuniform(std::string uni, GLint x, GLint y, GLint z, GLint w) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform4i_g(point, x, y, z, w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4i_g(*uf, x, y, z, w);
	return true;
}
//unsigned int overloads
bool Shader::setuniform(std::string uni, GLuint n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform1ui_g(point, n);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1ui_g(*uf, n);
	return true;
}
bool Shader::setuniform(std::string uni, GLuint x, GLuint y) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform2ui_g(point, x, y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2ui_g(*uf, x, y);
	return true;
}
bool Shader::setuniform(std::string uni, GLuint x, GLuint y, GLuint z) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform3ui_g(point, x, y, z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3ui_g(*uf, x, y, z);
	return true;
}
bool Shader::setuniform(std::string uni, GLuint x, GLuint y, GLuint z, GLuint w) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform4ui_g(point, x, y, z, w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4ui_g(*uf, x, y, z, w);
	return true;
}
//float overloads
bool Shader::setuniform(std::string uni, GLfloat n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform1f_g(point, n);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1f_g(*uf, n);
	return true;
}
bool Shader::setuniform(std::string uni, vec2 n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform2f_g(point, n.x, n.y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2f_g(*uf, n.x, n.y);
	return true;
}
bool Shader::setuniform(std::string uni, vec3 n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform3f_g(point, n.x, n.y, n.z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3f_g(*uf, n.x, n.y, n.z);
	return true;
}
bool Shader::setuniform(std::string uni, vec4 n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform4f_g(point, n.x, n.y, n.z, n.w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4f_g(*uf, n.x, n.y, n.z, n.w);
	return true;
}
//double overloads
bool Shader::setuniform(std::string uni, GLdouble n) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform1d_g(point, n);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1d_g(*uf, n);
	return true;
}
bool Shader::setuniform(std::string uni, GLdouble x, GLdouble y) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform2d_g(point, x, y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2d_g(*uf, x, y);
	return true;
}
bool Shader::setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform3d_g(point, x, y, z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3d_g(*uf, x, y, z);
	return true;
}
bool Shader::setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			glUniform4d_g(point, x, y, z, w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4d_g(*uf, x, y, z, w);
	return true;
}


//array overloads


//stride is number of elements in a single element which count represents, maxes out at 4
bool Shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLfloat* value) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1fv_g(point, count, value);
				break;
			case 2:
				glUniform2fv_g(point, count, value);
				break;
			case 3:
				glUniform3fv_g(point, count, value);
				break;
			case 4:
				glUniform4fv_g(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1fv_g(*uf, count, value);
		break;
	case 2:
		glUniform2fv_g(*uf, count, value);
		break;
	case 3:
		glUniform3fv_g(*uf, count, value);
		break;
	case 4:
		glUniform4fv_g(*uf, count, value);
		break;
	}
	return true;
}
//stride is number of elements in a single element which count represents, maxes out at 4
bool Shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLint* value) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1iv_g(point, count, value);
				break;
			case 2:
				glUniform2iv_g(point, count, value);
				break;
			case 3:
				glUniform3iv_g(point, count, value);
				break;
			case 4:
				glUniform4iv_g(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1iv_g(*uf, count, value);
		break;
	case 2:
		glUniform2iv_g(*uf, count, value);
		break;
	case 3:
		glUniform3iv_g(*uf, count, value);
		break;
	case 4:
		glUniform4iv_g(*uf, count, value);
		break;
	}
	return true;
}
//stride is number of elements in a single element which count represents, maxes out at 4
bool Shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLuint* value) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1uiv_g(point, count, value);
				break;
			case 2:
				glUniform2uiv_g(point, count, value);
				break;
			case 3:
				glUniform3uiv_g(point, count, value);
				break;
			case 4:
				glUniform4uiv_g(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1uiv_g(*uf, count, value);
		break;
	case 2:
		glUniform2uiv_g(*uf, count, value);
		break;
	case 3:
		glUniform3uiv_g(*uf, count, value);
		break;
	case 4:
		glUniform4uiv_g(*uf, count, value);
		break;
	}
	return true;
}

bool Shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLdouble* value) {
	GLint* uf = uniform_map.get(uni);
	if (uf == nullptr) {
		GLint point = glGetUniformLocation_g(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1dv_g(point, count, value);
				break;
			case 2:
				glUniform2dv_g(point, count, value);
				break;
			case 3:
				glUniform3dv_g(point, count, value);
				break;
			case 4:
				glUniform4dv_g(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1dv_g(*uf, count, value);
		break;
	case 2:
		glUniform2dv_g(*uf, count, value);
		break;
	case 3:
		glUniform3dv_g(*uf, count, value);
		break;
	case 4:
		glUniform4dv_g(*uf, count, value);
		break;
	}
	return true;
}

void Shader::compile(const char* vertex_file, const char* fragment_file) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader_g(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader_g(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	std::cout << "Compiling vertex shader" << std::endl;
	char const* VertexSourcePointer = vertex_file;
	glShaderSource_g(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader_g(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv_g(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv_g(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog_g(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	std::cout << "Compiling fragment shader" << std::endl;;
	char const* FragmentSourcePointer = fragment_file;
	glShaderSource_g(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader_g(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv_g(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv_g(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog_g(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	std::cout << "Linking program" << std::endl;
	GLuint ProgramID = glCreateProgram_g();
	glAttachShader_g(ProgramID, VertexShaderID);
	glAttachShader_g(ProgramID, FragmentShaderID);
	glLinkProgram_g(ProgramID);

	// Check the program
	glGetProgramiv_g(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv_g(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog_g(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader_g(ProgramID, VertexShaderID);
	glDetachShader_g(ProgramID, FragmentShaderID);

	glDeleteShader_g(VertexShaderID);
	glDeleteShader_g(FragmentShaderID);
	program = ProgramID;
}
void Shader::compile(const std::string vert_path, const std::string frag_path) {
	std::ifstream f;
	f.open(vert_path);
	std::stringstream ver_stream;
	ver_stream << f.rdbuf();
	std::string vstr = ver_stream.str();
	const char* vertex_file = vstr.c_str();
	f.close();
	f.open(frag_path);
	std::stringstream fra_stream;
	fra_stream << f.rdbuf();
	std::string fstr = fra_stream.str();
	const char* fragment_file = fstr.c_str();

	// Create the shaders
	GLuint VertexShaderID = glCreateShader_g(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader_g(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	std::cout << "Compiling vertex shader " << vert_path << std::endl;
	char const* VertexSourcePointer = vertex_file;
	glShaderSource_g(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader_g(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv_g(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv_g(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog_g(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	std::cout << "Compiling fragment shader " << frag_path << std::endl;;
	char const* FragmentSourcePointer = fragment_file;
	glShaderSource_g(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader_g(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv_g(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv_g(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog_g(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	std::cout << "Linking program" << std::endl;
	GLuint ProgramID = glCreateProgram_g();
	glAttachShader_g(ProgramID, VertexShaderID);
	glAttachShader_g(ProgramID, FragmentShaderID);
	glLinkProgram_g(ProgramID);

	// Check the program
	glGetProgramiv_g(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv_g(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog_g(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader_g(ProgramID, VertexShaderID);
	glDetachShader_g(ProgramID, FragmentShaderID);

	glDeleteShader_g(VertexShaderID);
	glDeleteShader_g(FragmentShaderID);
	program = ProgramID;
}