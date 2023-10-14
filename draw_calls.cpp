#include "g_engine_2d.h"

//2d drawing functions

void EngineNewGL::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	buffer_2d.push_back({ x1, y1 });
	buffer_2d.push_back({ x2, y2 });
	buffer_2d.push_back({ x3, y3 });
	glUseProgram_g(shader_triangle2d);
	glUniform4f_g(coloruniform_tri, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Triangle);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	//glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
	//glEnableVertexAttribArray_g(0);



	glDrawArrays(GL_TRIANGLES, 0, buffer_2d.size());
	buffer_2d.clear();
	//glDisableVertexAttribArray_g(0);
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);


}

void EngineNewGL::drawTriangles() {
	glUseProgram_g(shader_triangle2d);
	glUniform4f_g(coloruniform_tri, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Triangle);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, buffer_2d.size());
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
}

//draws a point
void EngineNewGL::drawPoint(float x1, float y1) {
	glEnable(GL_PROGRAM_POINT_SIZE);
	buffer_2d.push_back({ x1, y1 });

	glUseProgram_g(shader_point);
	glUniform4f_g(coloruniform_point, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Points);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, buffer_2d.size());
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

//draws points from the buffer_2d
void EngineNewGL::drawPoints() {

	glEnable(GL_PROGRAM_POINT_SIZE);
	glUseProgram_g(shader_point);
	glUniform4f_g(coloruniform_point, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Points);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_POINTS, 0, buffer_2d.size());
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

//draws a circle
void EngineNewGL::drawCircle(float x, float y, float r) {
	//use a seperate shader from points
	float x1, y1;
	for (float ang = 0; ang < 360; ang += 0.5f) {
		x1 = float(r * cos(ang * M_PI / 180) + x);
		y1 = float(r * sin(ang * M_PI / 180) + y);

		buffer_2d.push_back({ x1, y1 });
		buffer_2d.push_back({ x, y });
	}
	drawLines(1.0f);

}
//draws a quad, w and h have to be positive
void EngineNewGL::drawQuad(float x, float y, float w, float h) {
	//triangle 1
	buffer_2d.push_back({ x, y });
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x, y + h });
	//triangle 2
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x + w, y + h });
	buffer_2d.push_back({ x, y + h });
	//drawing triangles
	glUseProgram_g(shader_triangle2d);
	glUniform4f_g(coloruniform_tri, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Triangle);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, buffer_2d.size());
	//cleanup
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
}

//draws quads from buffer_2d
void EngineNewGL::drawQuads() {
	//drawing triangles
	glUseProgram_g(shader_triangle2d);
	glUniform4f_g(coloruniform_tri, draw_color.x, draw_color.y, draw_color.z, draw_color.w);
	glBindVertexArray_g(VAO_Triangle);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, buffer_2d.size());
	//cleanup
	buffer_2d.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
}


//vertice functions
void EngineNewGL::addQuad(float x, float y, float w, float h) {
	//triangle 1
	buffer_2d.push_back({ x, y });
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x, y + h });
	//triangle 2
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x + w, y + h });
	buffer_2d.push_back({ x, y + h });
}
void EngineNewGL::addTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	buffer_2d.push_back({ x1, y1 });
	buffer_2d.push_back({ x2, y2 });
	buffer_2d.push_back({ x3, y3 });
}


void EngineNewGL::add2DPoint(float x, float y) {
	buffer_2d.push_back({ x, y });
}
void EngineNewGL::add2DPoints(std::vector<vec2> points) {
	buffer_2d.insert(buffer_2d.end(), points.begin(), points.end());
}