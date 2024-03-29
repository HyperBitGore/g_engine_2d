#pragma once
#include "image_loader.hpp"

//switch to using multiple buffers so we can use all of the texture units on the gpu, but also have to dynamically generate the 
//https://www.khronos.org/opengl/wiki/Texture
	//-read the glsl binding section
//https://learnopengl.com/Getting-started/Transformations
class imagerenderer {
private:
	struct ivertex{
		float x;
		float y;
		float uvx;
		float uvy;
		float rot;
		float rotx;
		float roty;
	};
	std::vector<ivertex> vertexs;
	Shader shader;
	GLuint vao;
	GLuint vertex_buffer;
	GLuint allocated;
public:
	imagerenderer(size_t w, size_t h);
	void addImageVertex(vec2 pos, vec2 dimensions);
	void addImageVertex(vec2 pos, vec2 dimensions, float rot);
	void addImageVertex(vec2 pos, vec2 dimensions, vec4 uvs, float rot);
	void drawBuffer(IMG img);
	void drawBuffer(GLuint texture);
	void drawImage(IMG img, vec2 pos, vec2 dimensions);
	void drawImage(IMG img, vec2 pos, vec2 dimensions, vec4 uvs);
	void drawImageRotated(IMG img,vec2 pos, vec2 dimensions, float rot);
	void drawTexture(GLuint texture, vec2 pos, vec2 dimensions);
	void drawTexture(GLuint texture, vec2 pos, vec2 dimensions, vec4 uvs);
	void drawTextureRotated(GLuint texture, vec2 pos, vec2 dimensions, float rot);
};

//https://open.gl/framebuffers
//https://www.youtube.com/watch?v=QQ3jr-9Rc1o
class DrawPass {
	private:
		GLuint color_buffer;
		GLuint depth_buffer; //also stencil buffer
		GLuint texture;
		GLenum attach;
		GLsizei w;
		GLsizei h;
	public:
		DrawPass(GLsizei width, GLsizei height, GLenum attach) {
			w = width;
			h = height;
			glGenFramebuffers_g(1, &color_buffer);
			glBindFramebuffer_g(GL_FRAMEBUFFER, color_buffer);
			//glCreateTextures_g(GL_TEXTURE_2D, 1, &texture);
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			
			//creating render buffer
			glGenRenderbuffers_g(1, &depth_buffer);
			glBindRenderbuffer_g(GL_RENDERBUFFER, depth_buffer);
			glRenderbufferStorage_g(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer_g(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
			glFramebufferTexture2D_g(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			GLuint ret = glCheckFramebufferStatus_g(GL_FRAMEBUFFER);
			if (ret != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer failed creation!\n";
				std::cout << ret << "\n";
			}
			glBindFramebuffer_g(GL_FRAMEBUFFER, 0);

			this->attach = attach;
		}
		~DrawPass() {
			glDeleteFramebuffers_g(1, &color_buffer);
			glDeleteRenderbuffers_g(1, &depth_buffer);
			glDeleteTextures(1, &texture);
		}
		void bind() {
			glBindFramebuffer_g(GL_FRAMEBUFFER, color_buffer);
			//glViewport(0, 0, 640, 480);
		}
		void unbind() {
			glBindFramebuffer_g(GL_FRAMEBUFFER, 0);
		}
		//doesn't assume the framebuffer is binded
		void clear(){
			glBindFramebuffer_g(GL_FRAMEBUFFER, color_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer_g(GL_FRAMEBUFFER, 0);
		}
		GLuint getTexture() {
			return texture;
		}
};