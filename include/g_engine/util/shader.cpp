#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <stdexcept>

void gore::shader::bind() {
	glUseProgram(program);
}

bool gore::shader::setuniform(std::string uni, GLint n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform1i(point, n);
			uniform_map.insert(uni, point);
			return true;
		}
		return false;
	}
	glUniform1i(*uf, n);
	return true;
}

bool gore::shader::setuniform(std::string uni, GLint x, GLint y) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform2i(point, x, y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2i(*uf, x, y);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLint x, GLint y, GLint z) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform3i(point, x, y, z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3i(*uf, x, y, z);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLint x, GLint y, GLint z, GLint w) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform4i(point, x, y, z, w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4i(*uf, x, y, z, w);
	return true;
}

bool gore::shader::setuniform(std::string uni, GLsizei count, const GLint* values) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform1iv(point, count, values);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1iv(*uf, count, values);
	return true;
}

//unsigned int overloads
bool gore::shader::setuniform(std::string uni, GLuint n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform1ui(point, n);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1ui(*uf, n);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLuint x, GLuint y) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform2ui(point, x, y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2ui(*uf, x, y);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLuint x, GLuint y, GLuint z) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform3ui(point, x, y, z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3ui(*uf, x, y, z);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLuint x, GLuint y, GLuint z, GLuint w) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform4ui(point, x, y, z, w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4ui(*uf, x, y, z, w);
	return true;
}
//float overloads
bool gore::shader::setuniform(std::string uni, GLfloat n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform1f(point, n);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1f(*uf, n);
	return true;
}
bool gore::shader::setuniform(std::string uni, gore::vec2 n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform2f(point, n.x, n.y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2f(*uf, n.x, n.y);
	return true;
}
bool gore::shader::setuniform(std::string uni, gore::vec3 n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform3f(point, n.x, n.y, n.z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3f(*uf, n.x, n.y, n.z);
	return true;
}
bool gore::shader::setuniform(std::string uni, gore::vec4 n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform4f(point, n.x, n.y, n.z, n.w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4f(*uf, n.x, n.y, n.z, n.w);
	return true;
}
//double overloads
bool gore::shader::setuniform(std::string uni, GLdouble n) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform1d(point, n);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform1d(*uf, n);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLdouble x, GLdouble y) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform2d(point, x, y);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform2d(*uf, x, y);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform3d(point, x, y, z);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform3d(*uf, x, y, z);
	return true;
}
bool gore::shader::setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			glUniform4d(point, x, y, z, w);
			uniform_map.insert(uni, point);
		}
		return false;
	}
	glUniform4d(*uf, x, y, z, w);
	return true;
}


//array overloads


//stride is number of elements in a single element which count represents, maxes out at 4
bool gore::shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLfloat* value) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1fv(point, count, value);
				break;
			case 2:
				glUniform2fv(point, count, value);
				break;
			case 3:
				glUniform3fv(point, count, value);
				break;
			case 4:
				glUniform4fv(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1fv(*uf, count, value);
		break;
	case 2:
		glUniform2fv(*uf, count, value);
		break;
	case 3:
		glUniform3fv(*uf, count, value);
		break;
	case 4:
		glUniform4fv(*uf, count, value);
		break;
	}
	return true;
}
//stride is number of elements in a single element which count represents, maxes out at 4
bool gore::shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLint* value) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1iv(point, count, value);
				break;
			case 2:
				glUniform2iv(point, count, value);
				break;
			case 3:
				glUniform3iv(point, count, value);
				break;
			case 4:
				glUniform4iv(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1iv(*uf, count, value);
		break;
	case 2:
		glUniform2iv(*uf, count, value);
		break;
	case 3:
		glUniform3iv(*uf, count, value);
		break;
	case 4:
		glUniform4iv(*uf, count, value);
		break;
	}
	return true;
}
//stride is number of elements in a single element which count represents, maxes out at 4
bool gore::shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLuint* value) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1uiv(point, count, value);
				break;
			case 2:
				glUniform2uiv(point, count, value);
				break;
			case 3:
				glUniform3uiv(point, count, value);
				break;
			case 4:
				glUniform4uiv(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1uiv(*uf, count, value);
		break;
	case 2:
		glUniform2uiv(*uf, count, value);
		break;
	case 3:
		glUniform3uiv(*uf, count, value);
		break;
	case 4:
		glUniform4uiv(*uf, count, value);
		break;
	}
	return true;
}

bool gore::shader::setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLdouble* value) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			switch (stride) {
			case 1:
				glUniform1dv(point, count, value);
				break;
			case 2:
				glUniform2dv(point, count, value);
				break;
			case 3:
				glUniform3dv(point, count, value);
				break;
			case 4:
				glUniform4dv(point, count, value);
				break;
			}
			uniform_map.insert(uni, point);
		}
		return false;
	}
	switch (stride) {
	case 1:
		glUniform1dv(*uf, count, value);
		break;
	case 2:
		glUniform2dv(*uf, count, value);
		break;
	case 3:
		glUniform3dv(*uf, count, value);
		break;
	case 4:
		glUniform4dv(*uf, count, value);
		break;
	}
	return true;
}
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUniform.xhtml
bool gore::shader::setuniform(const std::string uni, const GLsizei count, const GLboolean transpose, gore::matrix& matrice) {
	GLint* uf = uniform_map.get(uni);
	this->bind();
	if (uf == nullptr) {
		GLint point = glGetUniformLocation(program, uni.c_str());
		if (point != -1) {
			uniform_map.insert(uni, point);
		} else {
			return false;
		}
		uf = uniform_map.get(uni);
	}
	switch (matrice.numColumns()) {
		case 2:
			switch (matrice.numRows()) {
				case 2:

				break;
				case 3:

				break;
				case 4:
				
				break;
			}
		break;
		case 3:
			switch (matrice.numRows()) {
				case 2:
				
				break;
				case 3:
				
				break;
				case 4:
				
				break;
			}
		break;
		case 4:
			switch (matrice.numRows()) {
				case 2:
				
				break;
				case 3:
				
				break;
				case 4:
					glUniformMatrix4fv(*uf, count, transpose, matrice.data());
				break;
			}
		break;
		default:
		return false;
	}
	return true;
}

void gore::shader::compile(const char* vertex_file, const char* fragment_file) {
	// precompile pass, for our custom shader extensions
	std::string vertex_string = textureUnitSub(vertex_file);
	std::string fragment_string = textureUnitSub(fragment_file);
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	std::cout << "Compiling vertex shader" << std::endl;
	char const* VertexSourcePointer = vertex_string.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	std::cout << "Compiling fragment shader" << std::endl;;
	char const* FragmentSourcePointer = fragment_string.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	std::cout << "Linking program" << std::endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	program = ProgramID;
}
void gore::shader::compile(const std::string vert_path, const std::string frag_path) {
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
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	std::string vertex_string = textureUnitSub(vertex_file);
	std::string fragment_string = textureUnitSub(fragment_file);
	std::cout << "Compiling vertex shader " << vert_path << std::endl;
	char const* VertexSourcePointer = vertex_string.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	std::cout << "Compiling fragment shader " << frag_path << std::endl;;
	char const* FragmentSourcePointer = fragment_string.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	std::cout << "Linking program" << std::endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	program = ProgramID;
}
// subs out TEXTURE_UNIT in shader code with texture units on system
std::string gore::shader::textureUnitSub (std::string shader_code) {
	int32_t texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	size_t index = shader_code.find("TEXTURE_UNIT");
	if (index != std::string::npos) {
		std::string tex_string = std::to_string(texture_units);
		shader_code.replace(index, 12, tex_string);
	}
	index = shader_code.find("TEXTURE_SWITCH");
	if (index != std::string::npos) {
		const size_t token_length = 14;
		size_t argument_index = index + token_length;
		while (argument_index < shader_code.size() &&
			std::isspace(static_cast<unsigned char>(shader_code[argument_index]))) {
			++argument_index;
		}
		const size_t texture_array_start = argument_index;
		while (argument_index < shader_code.size() &&
			(std::isalnum(static_cast<unsigned char>(shader_code[argument_index])) ||
				shader_code[argument_index] == '_')) {
			++argument_index;
		}
		const std::string texture_array_name =
			shader_code.substr(texture_array_start, argument_index - texture_array_start);
		while (argument_index < shader_code.size() &&
			std::isspace(static_cast<unsigned char>(shader_code[argument_index]))) {
			++argument_index;
		}
		const size_t texcoord_start = argument_index;
		while (argument_index < shader_code.size() &&
			(std::isalnum(static_cast<unsigned char>(shader_code[argument_index])) ||
				shader_code[argument_index] == '_')) {
			++argument_index;
		}
		const std::string texcoord_name =
			shader_code.substr(texcoord_start, argument_index - texcoord_start);
		if (texture_array_name.empty() || texcoord_name.empty()) {
			throw std::runtime_error(
				"TEXTURE_SWITCH requires a sampler array and texture coordinate name");
		}
		std::string switch_string;
		for (int32_t unit = 0; unit < texture_units; ++unit) {
			switch_string += "case " + std::to_string(unit) +
				"u: color = texture(" + texture_array_name + "[" +
				std::to_string(unit) + "], " + texcoord_name + "); break;\n";
		}
		shader_code.replace(index, argument_index - index, switch_string);
	}

	return shader_code;
}