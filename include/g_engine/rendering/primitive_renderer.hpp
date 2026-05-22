#pragma once
#include "image_renderer.hpp"
#include "renderer.hpp"

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

class trianglerenderer : public renderer<gore::vec2> {
	private:
	void shader_setup() override {
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
	}
	public:
	trianglerenderer(uint32_t width, uint32_t height);
	// copy
	trianglerenderer(const trianglerenderer& tr);
	void setColor(vec4 color);
	void addTriangle(vec2 v1, vec2 v2, vec2 v3);
	void drawTriangle(vec2 v1, vec2 v2, vec2 v3);
	void addQuad(vec2 pos, float w, float h);
	void drawQuad(vec2 pos, float w, float h);
	void drawCircleFilled(vec2 p, float r);
	void addCircleFilled(vec2 p, float r);
};

class pointrenderer : public renderer<gore::vec2> {
	private:
	void shader_setup() override {
		draw_arrays_mode = GL_POINTS;
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
	}
	public:
	pointrenderer(uint32_t width, uint32_t height);
	// copy
	pointrenderer(const pointrenderer& pr);
	void setColor(vec4 color);
	void addPoint(vec2 p);
	void drawPoint(vec2 p);
};

class linerenderer : public renderer<gore::vec2> {
	private:
	void shader_setup() override {
		draw_arrays_mode = GL_LINES;
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
	}
	public:
	void drawBuffer() override;
	linerenderer(uint32_t width, uint32_t height);
	// copy
	linerenderer(const linerenderer& lr);
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