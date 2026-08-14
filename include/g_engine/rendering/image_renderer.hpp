#pragma once
#include "../img_loading/image_loader.hpp"
#include "renderer.hpp"
#include "../util/gl_tagger.hpp"
#include "texture_unit_manager.hpp"
#include <stdexcept>
#include <cstdint>
#include <cstddef>

//switch to using multiple buffers so we can use all of the texture units on the gpu, but also have to dynamically generate the 
//https://www.khronos.org/opengl/wiki/Texture
	//-read the glsl binding section
//https://learnopengl.com/Getting-started/Transformations
namespace gore {
	struct image_render_vertex {
		float x;
		float y;
		float uvx;
		float uvy;
		float rot;
		float rotx;
		float roty;
		uint32_t texture_unit;
	};
// issue with the triangles getting wrong textures is probably a memory copying issue??
// switch to using bindless textures
class imagerenderer : public renderer<imagerenderer, image_render_vertex> {
protected:
	friend class renderer<imagerenderer, image_render_vertex>;
	imagerenderer () = default;
	bindless_texture_manager texture_map;
	GLuint texture_ssbo;
	void setTextureSamplers ();
	void shader_setup() override {
		gl_function_tagger tags({
			"glBindVertexArray",
			"glBindBuffer",
			"glEnableVertexAttribArray",
			"glVertexAttribPointer",
			"glVertexAttribIPointer",
			"glGenBuffers",
			"glBindBufferBase",
			"glBufferData",
			"glBufferSubData",
			"glDrawArrays",
			"glDrawArraysExt",
			"glGetTextureHandleARB",
			"glMakeTextureHandleResidentARB",
			"glMakeTextureHandleNonResidentARB",
			"glIsTextureHandleResidentARB"
		});
		try {
			tags.hardwareSupports();
		} catch (render_function_not_supported& e) {
			std::cout << e.what() << "do smth\n";
		}
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(image_render_vertex), (void*)0); //position
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(image_render_vertex), (void*)(sizeof(float) * 2)); //uv
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(image_render_vertex), (void*)(sizeof(float) * 4)); //rotation
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(image_render_vertex), (void*)(sizeof(float) * 5)); //rotation point
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(image_render_vertex), (void*)offsetof(image_render_vertex, texture_unit)); // texture unit
		updateView(0.0f, 0.0f, 1.0f);
		updateDimensions(this->width, this->height);
		glGenBuffers(1, &texture_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, texture_ssbo);
	}
	imagerenderer(size_t w, size_t h);
public:
	void addImageVertex(GLuint texture, gore::vec2 pos, gore::vec2 dimensions);
	void addImageVertex(GLuint texture, gore::vec2 pos, gore::vec2 dimensions, float rot);
	void addImageVertex(GLuint texture, gore::vec2 pos, gore::vec2 dimensions, gore::vec4 uvs, float rot);
	void drawBuffer() override;
	void drawImage(const gore::IMG& img, gore::vec2 pos, gore::vec2 dimensions);
	void drawImage(const gore::IMG& img, gore::vec2 pos, gore::vec2 dimensions, gore::vec4 uvs);
	void drawImageRotated(const IMG& img,gore::vec2 pos, gore::vec2 dimensions, float rot);
	void drawTexture(GLuint texture, gore::vec2 pos, gore::vec2 dimensions);
	void drawTexture(GLuint texture, gore::vec2 pos, gore::vec2 dimensions, gore::vec4 uvs);
	void drawTextureRotated(GLuint texture, gore::vec2 pos, gore::vec2 dimensions, float rot);
};

class grayscalerenderer : public imagerenderer {
	protected:
	friend class renderer<imagerenderer, image_render_vertex>;
	void shader_setup() override {
		gl_function_tagger tags({
			"glGenVertexArrays",
			"glGenBuffers",
			"glBindVertexArray",
			"glBindBuffer",
			"glEnableVertexAttribArray",
			"glVertexAttribPointer",
			"glVertexAttribIPointer",
			"glBindBufferBase",
			"glBufferData",
			"glBufferSubData",
			"glDrawArrays",
			"glGetTextureHandleARB",
			"glMakeTextureHandleResidentARB",
			"glMakeTextureHandleNonResidentARB",
			"glIsTextureHandleResidentARB"
		});
		try {
			tags.hardwareSupports();
		} catch (render_function_not_supported& e) {

		}
		updateView(0.0f, 0.0f, 1.0f);
		updateDimensions(this->width, this->height);
	}
	grayscalerenderer(size_t w, size_t h);
	public:
	void setWithAlpha(bool withAlpha) {
		shader.bind();
		shader.setuniform("withAlpha", withAlpha);
	}
};

//https://open.gl/framebuffers
//https://www.youtube.com/watch?v=QQ3jr-9Rc1o
class drawpass {
	private:
		GLuint color_buffer;
		GLuint depth_buffer; //also stencil buffer
		GLuint texture;
		GLenum attach;
		GLsizei w;
		GLsizei h;
	public:
		// this does nothing, dont use uninited drawpasses, u will get fucking huge errors!
		drawpass () {

		}
		drawpass(GLsizei width, GLsizei height, GLenum attach) {
			if (width == 0 || height == 0) {
				throw std::runtime_error("Can't construct a drawpass with width or height set to zero!");
			}
			w = width;
			h = height;
			glGenFramebuffers(1, &color_buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, color_buffer);

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			
			//creating render buffer
			glGenRenderbuffers(1, &depth_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			GLuint ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (ret != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer failed creation!\n";
				std::cout << ret << "\n";
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			this->attach = attach;
		}
		// copy
		drawpass (const drawpass& dr) {
			this->attach = dr.attach;
			this->w = dr.w;
			this->h = dr.h;
			glGenFramebuffers(1, &color_buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, color_buffer);

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			
			//creating render buffer
			glGenRenderbuffers(1, &depth_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			GLuint ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (ret != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer failed creation, during copy operation!\n";
				std::cout << ret << "\n";
			}
			glBindFramebuffer(GL_READ_FRAMEBUFFER, dr.color_buffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, color_buffer);
			glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		~drawpass() {
			glDeleteFramebuffers(1, &color_buffer);
			glDeleteRenderbuffers(1, &depth_buffer);
			glDeleteTextures(1, &texture);
		}
		void bind() {
			glBindFramebuffer(GL_FRAMEBUFFER, color_buffer);
		}
		void unbind() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		//doesn't assume the framebuffer is binded
		void clear(){
			glBindFramebuffer(GL_FRAMEBUFFER, color_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		GLuint getTexture() {
			return texture;
		}
		GLuint getColorBuffer () {
			return color_buffer;
		}
		void resize (uint32_t width, uint32_t height) {
			this->w = width;
			this->h = height;
			bind();
			glViewport(0, 0, width, height);
			// Reallocate the texture
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			// Reallocate the depth/stencil renderbuffer
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

			// Re-attach them (optional, in case you're paranoid about driver bugs)
			glBindFramebuffer(GL_FRAMEBUFFER, color_buffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
			unbind();
		}
};
}
