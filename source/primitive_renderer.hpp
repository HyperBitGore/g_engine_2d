#pragma once
#include "image_renderer.hpp"

class Line {
private:
	
public:
	vec2 p1;
	vec2 p2;
	Line() {
		
	}
	Line(vec2 v1, vec2 v2) {
		p1 = v1;
		p2 = v2;
	}
	Line(const Line& l) {
		p1 = l.p1;
		p2 = l.p2;
	}
};

class PrimitiveRenderer {
private:
	std::vector<vec2> vertexs;
	GLuint vertex_buffer;
	GLuint allocated;
	Shader triangle_shader;
	GLuint triangle_vao;
	Shader point_shader;
	GLuint point_vao;
	Shader line_shader;
	GLuint line_vao;
public:
	//use to initialize shaders
	PrimitiveRenderer(GLuint sw, GLuint sh);
	//sets color for drawing
	void setColor(vec4 color);
	//triangles
	void addTriangle(vec2 v1, vec2 v2, vec2 v3);
	void drawTriangle(vec2 v1, vec2 v2, vec2 v3);
	void drawBufferTriangle();
	//quads
	void addQuad(vec2 pos, float w, float h);
	void drawQuad(vec2 pos, float w, float h);
	void drawBufferQuad();
	//points
	void addPoint(vec2 p);
	void drawPoint(vec2 p);
	void drawBufferPoint();
	//lines
	void addLine(vec2 p1, vec2 p2);
	void drawLine(vec2 p1, vec2 p2);
	void drawBufferLine();
	void setLineWidth(float l);
	//this add points to buffer, they are used for lines so make sure to call drawBufferLine when these are used
	void linearBezier(vec2 p1, vec2 p2);
	void quadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv);
	void cubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv);
	void circle(vec2 p, float r);
};