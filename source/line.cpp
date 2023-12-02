#include "g_engine_2d.h"


void EngineNewGL::addquadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx, ly;
		buffer_2d.push_back({ lx, ly });
		buffer_2d.push_back({ x, y });
		lx = x;
		ly = y;
	}
}

//draw a bezier curve
//could probably throw this onto the gpu as a seperate shader, if I need to draw a gazillion curves
void EngineNewGL::quadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx, ly;
		buffer_2d.push_back({ lx, ly });
		buffer_2d.push_back({ x, y });
		lx = x;
		ly = y;
	}
	drawLines(0.5f);
	//drawPoints();
}

void EngineNewGL::cubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv) {
	float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * t1 * p1.x + 3 * t1 * t1 * t * p2.x + 3 * t1 * t2 * p3.x + t2 * t * p4.x;
		float y = t1 * t1 * t1 * p1.y + 3 * t1 * t1 * t * p2.y + 3 * t1 * t2 * p3.y + t2 * t * p4.y;
		(i == 0) ? lx = x, ly = y : lx, ly;
		buffer_2d.push_back({ lx, ly });
		buffer_2d.push_back({ x, y });
		lx = x;
		ly = y;
	}
	drawLines(0.5f);
}
//draw line of points, using a linear bezier curve
void EngineNewGL::drawLinePoints(vec2 p1, vec2 p2) {
	int subdiv = (int)std::abs(p1.x - p2.x) + (int)std::abs(p1.y - p2.y);
	float step = 1.0f / (float)subdiv;
	//passing point or not reaching it, need to re-examine how many subdiv to do
	for (int i = 0; i < subdiv; i++) {
		float t = i * step;
		float x = (1 - t) * p1.x + t * p2.x;
		float y = (1 - t) * p1.y + t * p2.y;
		buffer_2d.push_back({ x, y });
	}
	drawPoints();
}

void EngineNewGL::addLinePoints(vec2 p1, vec2 p2) {
	int subdiv = (int)std::abs(p1.x - p2.x) + (int)std::abs(p1.y - p2.y);
	float step = 1.0f / (float)subdiv;
	//passing point or not reaching it, need to re-examine how many subdiv to do
	for (int i = 0; i < subdiv; i++) {
		float t = i * step;
		float x = (1 - t) * p1.x + t * p2.x;
		float y = (1 - t) * p1.y + t * p2.y;
		buffer_2d.push_back({ x, y });
	}
}

//draws line from buffer_2d
void EngineNewGL::drawLines(float width) {
	glLineWidth(width);
	//glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//write seperate shader
	glUseProgram_g(shader_line);
	glUniform4f_g(coloruniform_line, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Line);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, buffer_2d.size());
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	//glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}


//draws a line
void EngineNewGL::drawLine(float x1, float y1, float x2, float y2, float width) {
	glLineWidth(width);
	//glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//write seperate shader
	glUseProgram_g(shader_line);
	glUniform4f_g(coloruniform_line, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	buffer_2d.push_back({ x1, y1 });
	buffer_2d.push_back({ x2, y2 });


	glBindVertexArray_g(VAO_Line);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, buffer_2d.size());
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	//glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}