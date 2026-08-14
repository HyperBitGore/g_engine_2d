#pragma once
#include "renderer.hpp"
#include "../util/gl_tagger.hpp"

namespace gore {

class line {
public:
	vec2 p1;
	vec2 p2;
	line() {
		
	}
	line(vec2 v1, vec2 v2) {
		p1 = v1;
		p2 = v2;
	}
	line(const line& l) {
		p1 = l.p1;
		p2 = l.p2;
	}
};

class trianglerenderer : public renderer<trianglerenderer, gore::vec2> {
	private:
	friend class renderer<trianglerenderer, gore::vec2>;
	void shader_setup() override {
		gl_function_tagger tags({
			"glBindVertexArray",
			"glBindBuffer",
			"glEnableVertexAttribArray",
			"glVertexAttribPointer",
			"glBufferData",
			"glBufferSubData",
			"glDrawArrays"
		});
		try {
			tags.hardwareSupports();
		} catch (render_function_not_supported& e) {

		}
		shader.bind();
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
		updateDimensions(this->width, this->height);
		updateView(0.0f, 0.0f, 1.0f);
	}
	trianglerenderer(uint32_t width, uint32_t height);
	public:
	void setColor(vec4 color);
	void addTriangle(vec2 v1, vec2 v2, vec2 v3);
	void drawTriangle(vec2 v1, vec2 v2, vec2 v3);
	void addQuad(vec2 pos, float w, float h);
	void drawQuad(vec2 pos, float w, float h);
	void drawCircleFilled(vec2 p, float r);
	void addCircleFilled(vec2 p, float r);
};

class pointrenderer : public renderer<pointrenderer, gore::vec2> {
	private:
	friend class renderer<pointrenderer, gore::vec2>;
	void shader_setup() override {
		gl_function_tagger tags({
			"glBindVertexArray",
			"glBindBuffer",
			"glEnableVertexAttribArray",
			"glVertexAttribPointer",
			"glBufferData",
			"glBufferSubData",
			"glDrawArrays"
		});
		try {
			tags.hardwareSupports();
		} catch (render_function_not_supported& e) {

		}
		draw_arrays_mode = GL_POINTS;
		shader.bind();
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
		updateDimensions(this->width, this->height);
		updateView(0.0f, 0.0f, 1.0f);
	}
	pointrenderer(uint32_t width, uint32_t height);
	public:
	void setColor(vec4 color);
	void addPoint(vec2 p);
	void drawPoint(vec2 p);
};

class linerenderer : public renderer<linerenderer, gore::vec2> {
	private:
	friend class renderer<linerenderer, gore::vec2>;
	void shader_setup() override {
		gl_function_tagger tags({
			"glBindVertexArray",
			"glBindBuffer",
			"glEnableVertexAttribArray",
			"glVertexAttribPointer",
			"glBufferData",
			"glBufferSubData",
			"glDrawArrays"
		});
		try {
			tags.hardwareSupports();
		} catch (render_function_not_supported& e) {

		}
		draw_arrays_mode = GL_LINES;
		shader.bind();
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
		updateDimensions(this->width, this->height);
		updateView(0.0f, 0.0f, 1.0f);
	}
	linerenderer(uint32_t width, uint32_t height);
	public:
	void drawBuffer() override;
	void setColor(vec4 color);
	void addLine(vec2 p1, vec2 p2);
	void drawLine(vec2 p1, vec2 p2);
	void setLineWidth(float l);
	void drawLinearBezier(vec2 p1, vec2 p2);
	void drawQuadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv);
	void drawCubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv);
	void addLinearBezier(vec2 p1, vec2 p2);
	void addQuadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv);
	void addCubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv);
	void addCircleOutline(vec2 p, float r, uint32_t segments);
	void drawCircleOutline(vec2 p, float r, uint32_t segments);
};
}