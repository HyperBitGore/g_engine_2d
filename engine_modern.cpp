#include "g_engine_2d.h"



//2d drawing functions

void EngineNewGL::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	buffer_2d.push_back({ x1, y1});
	buffer_2d.push_back({ x2, y2});
	buffer_2d.push_back({ x3, y3});
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
	buffer_2d.push_back({ x1, y1});

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
		float x = t1 * t1 * t1 * p1.x + 3*t1*t1*t*p2.x + 3*t1*t2*p3.x + t2*t*p4.x;
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

//draws a circle
void EngineNewGL::drawCircle(float x, float y, float r) {
	//use a seperate shader from points
	float x1, y1;
	for (float ang = 0; ang < 360; ang += 0.5f) {
		x1 = float(r * cos(ang * M_PI / 180) + x);
		y1 = float(r * sin(ang * M_PI / 180) + y);

		buffer_2d.push_back({ x1, y1});
		buffer_2d.push_back({x, y});
	}
	drawLines(1.0f);

}
//draws a quad, w and h have to be positive
void EngineNewGL::drawQuad(float x, float y, float w, float h) {
	//triangle 1
	buffer_2d.push_back({ x, y});
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x, y+h });
	//triangle 2
	buffer_2d.push_back({ x+w, y });
	buffer_2d.push_back({ x + w, y+h });
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

//2d image drawing functions

//mass draws an image based on buffer_2d
void EngineNewGL::renderImgs(IMG img, bool blend) {
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glUseProgram_g(shader_img);
	glBindTextureUnit_g(img->pos, img->tex);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_img, "image_tex");
	glUniform1i_g(texuniform_img, img->pos);

	glBindVertexArray_g(VAO_Img);

	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, img_vertexs.size() * sizeof(img_vertex), &img_vertexs[0], GL_STATIC_DRAW);

	//glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	//glBufferData_g(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(vec2), &buffer_uv[0], GL_STATIC_DRAW);

	glDrawArrays_g(GL_TRIANGLES, 0, img_vertexs.size());


	img_vertexs.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
	if (blend) {
		glDisable(GL_BLEND);
	}
}


//rotates counter clockwise around top left point, angle is in raidans
//mass draws an image with rotations
void EngineNewGL::renderImgsRotated(IMG img, bool blend) {
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glUseProgram_g(shader_imgr);
	glBindTextureUnit_g(img->pos, img->tex);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_imgr, "image_tex");
	glUniform1i_g(texuniform_imgr, img->pos);
	GLuint screenu = glGetUniformLocation_g(shader_imgr, "screen"); //for some reason have to keep setting the screen data but it works so idc
	glUniform2f_g(screenu, (float)wind->getWidth(), (float)wind->getHeight());
	glBindVertexArray_g(VAO_Imgr);

	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, img_vertexs.size() * sizeof(img_vertex), &img_vertexs[0], GL_STATIC_DRAW);


	glDrawArrays_g(GL_TRIANGLES, 0, img_vertexs.size());


	img_vertexs.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
	if (blend) {
		glDisable(GL_BLEND);
	}
}


//utility type functions

bool EngineNewGL::updateWindow() {
	UpdateWindow(wind->getHwnd());
	if (!wind->ProcessMessage()) {
		std::cout << "Closing window\n";
		delete wind;
		return false;
	}
	begin_f = clock();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderFund();
	

	if (!SwapBuffers(dc_w))
	{
		FatalError("Failed to swap OpenGL buffers!");
	}
	end_f = clock();
	delta = end_f - begin_f;
	delta_f += end_f - begin_f;
	frames++;
	return true;
}


