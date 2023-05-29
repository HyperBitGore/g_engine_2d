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
	glEnable(GL_BLEND);
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
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}
//draws line from buffer_2d
void EngineNewGL::drawLines(float width) {
	glLineWidth(width);
	glEnable(GL_BLEND);
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
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

//2d image drawing functions
//
void EngineNewGL::renderImg(IMG img, float x, float y, float w, float h, bool blend) {
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//triangle 1
	buffer_2d.push_back({ x, y });
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x, y - h });
	//triangle 2
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x + w, y - h });
	buffer_2d.push_back({ x, y - h });
	//uv triangle 1
	buffer_uv.push_back({ 0, 0 });
	buffer_uv.push_back({ 1, 0 });
	buffer_uv.push_back({ 0, 1 });
	//uv triangle 2
	buffer_uv.push_back({ 1, 0 });
	buffer_uv.push_back({ 1, 1 });
	buffer_uv.push_back({ 0, 1 });
	

	
	glUseProgram_g(shader_img);
	glBindTextureUnit_g(img->pos, img->tex);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_img, "image_tex");
	glUniform1i_g(texuniform_img, img->pos);

	glBindVertexArray_g(VAO_Img);
	
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(vec2), &buffer_uv[0], GL_STATIC_DRAW);
	
	glDrawArrays_g(GL_TRIANGLES, 0, buffer_2d.size());
	

	buffer_2d.clear();
	buffer_uv.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
	if (blend) {
		glDisable(GL_BLEND);
	}
}
//mass draws an image based on buffer_2d
void EngineNewGL::renderImgs(IMG img) {
	glUseProgram_g(shader_img);
	glBindTextureUnit_g(img->pos, img->tex);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_img, "image_tex");
	glUniform1i_g(texuniform_img, img->pos);

	glBindVertexArray_g(VAO_Img);

	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(vec2), &buffer_uv[0], GL_STATIC_DRAW);

	glDrawArrays_g(GL_TRIANGLES, 0, buffer_2d.size());


	buffer_2d.clear();
	buffer_uv.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
}


//rotates counter clockwise around top left point, angle is in raidans
void EngineNewGL::renderImgRotated(IMG img, float x, float y, float w, float h, float ang, bool blend) {
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	int b = buffer_2d.size();
	//triangle 1
	buffer_2d.push_back({ x, y });
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x, y - h });
	//triangle 2
	buffer_2d.push_back({ x + w, y });
	buffer_2d.push_back({ x + w, y - h });
	buffer_2d.push_back({ x, y - h });

	for (size_t i = b; i < buffer_2d.size(); i++) {
		float out_x = buffer_2d[i].x / float(wind->getWidth());
		float out_y = buffer_2d[i].y / float(wind->getHeight());
		out_x = (out_x * 2.0f) - 1;
		out_y = (out_y * 2.0f) - 1;

		float rotx = buffer_2d[i].x / float(wind->getWidth());
		float roty = buffer_2d[i].y / float(wind->getHeight());
		rotx = (rotx * 2.0f) - 1;
		roty = (roty * 2.0f) - 1;


		out_x = (out_x - rotx);
		out_y = (out_y - roty);

		out_x = out_x * cos(ang) - out_y * sin(ang);
		out_y = out_y * cos(ang) + out_x * sin(ang);

		out_x += rotx;
		out_y += roty;
		buffer_2d[i].x = out_x;
		buffer_2d[i].y = out_y;
	}

	//uv triangle 1
	buffer_uv.push_back({ 0, 0 });
	buffer_uv.push_back({ 1, 0 });
	buffer_uv.push_back({ 0, 1 });
	//uv triangle 2
	buffer_uv.push_back({ 1, 0 });
	buffer_uv.push_back({ 1, 1 });
	buffer_uv.push_back({ 0, 1 });
	//rotations
	rotations.push_back(ang);
	rotations.push_back(ang);
	rotations.push_back(ang);
	rotations.push_back(ang);
	rotations.push_back(ang);
	rotations.push_back(ang);
	//rot_points
	rot_points.push_back({ x, y });
	rot_points.push_back({ x, y });
	rot_points.push_back({ x, y });
	rot_points.push_back({ x, y });
	rot_points.push_back({ x, y });
	rot_points.push_back({ x, y });

	glUseProgram_g(shader_imgr);
	glBindTextureUnit_g(img->pos, img->tex);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_imgr, "image_tex");
	glUniform1i_g(texuniform_imgr, img->pos);

	/*GLuint rot_point = glGetUniformLocation_g(shader_imgr, "rot_point");
	glUniform2f_g(rot_point, x, y);

	GLuint ang_uniform = glGetUniformLocation_g(shader_imgr, "ang");
	glUniform1f_g(ang_uniform, ang);*/

	glBindVertexArray_g(VAO_Imgr);

	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(vec2), &buffer_uv[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, rot_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, rotations.size() * sizeof(float), &rotations[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, rotpoint_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, rot_points.size() * sizeof(vec2), &rot_points[0], GL_STATIC_DRAW);

	glDrawArrays_g(GL_TRIANGLES, 0, buffer_2d.size());


	buffer_2d.clear();
	buffer_uv.clear();
	rotations.clear();
	rot_points.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
	if (blend) {
		glDisable(GL_BLEND);
	}
}

//mass draws an image with rotations
void EngineNewGL::renderImgsRotated(IMG img) {
	glUseProgram_g(shader_imgr);
	glBindTextureUnit_g(img->pos, img->tex);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_imgr, "image_tex");
	glUniform1i_g(texuniform_imgr, img->pos);

	glBindVertexArray_g(VAO_Imgr);

	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec2), &buffer_2d[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(vec2), &buffer_uv[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, rot_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, rotations.size() * sizeof(float), &rotations[0], GL_STATIC_DRAW);

	glBindBuffer_g(GL_ARRAY_BUFFER, rotpoint_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, rot_points.size() * sizeof(vec2), &rot_points[0], GL_STATIC_DRAW);

	glDrawArrays_g(GL_TRIANGLES, 0, buffer_2d.size());


	buffer_2d.clear();
	buffer_uv.clear();
	rotations.clear();
	rot_points.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
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


//https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
EngineNewGL::EngineNewGL(LPCWSTR window_name, int width, int height) {
	//function pointers
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

	//getting device context
	{
		// to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
		HWND dummy = CreateWindowExW(
			0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL);
		Assert(dummy && "Failed to create dummy window");

		HDC dc = GetDC(dummy);
		Assert(dc && "Failed to get device context for dummy window");

		PIXELFORMATDESCRIPTOR desc =
		{
			desc.nSize = sizeof(desc),
			desc.nVersion = 1,
			desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
			desc.iPixelType = PFD_TYPE_RGBA,
			desc.cColorBits = 32,
		};

		int format = ChoosePixelFormat(dc, &desc);
		if (!format)
		{
			FatalError("Cannot choose OpenGL pixel format for dummy window!");
		}

		int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
		Assert(ok && "Failed to describe OpenGL pixel format");

		// reason to create dummy window is that SetPixelFormat can be called only once for the window
		if (!SetPixelFormat(dc, format, &desc))
		{
			FatalError("Cannot set OpenGL pixel format for dummy window!");
		}

		HGLRC rc = wglCreateContext(dc);
		Assert(rc && "Failed to create OpenGL context for dummy window");

		ok = wglMakeCurrent(dc, rc);
		Assert(ok && "Failed to make current OpenGL context for dummy window");
		//now get wgl functions using the dummy context


		PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
			(PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
		const char* ext = wglGetExtensionsStringARB(dc);
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(rc);
		ReleaseDC(dummy, dc);
		DestroyWindow(dummy);
	}
	//create window
	wind = new Window(window_name, L"ENG1", width, height, 300, 300);
	in = new Input();
	dc_w = GetDC(wind->getHwnd());

	// set pixel format for OpenGL context
	{
		//https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
		int attrib[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			/* WGL_SWAP_EXCHANGE_ARB causes problems with window menu in fullscreen */
			WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,

			// uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
			//WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

			// uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
			//WGL_SAMPLE_BUFFERS_ARB, 1,
			//WGL_SAMPLES_ARB,        4, // 4x MSAA

			0
		};

		int format;
		UINT formats;
		if (!wglChoosePixelFormatARB(dc_w, attrib, NULL, 1, &format, &formats) || formats == 0)
		{
			FatalError("OpenGL does not support required pixel format!");
		}

		PIXELFORMATDESCRIPTOR desc;
		desc.nSize = sizeof(desc);
	//	desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		//desc.iPixelType = PFD_TYPE_RGBA;
		//desc.cColorBits = 32;


		int ok = DescribePixelFormat(dc_w, format, sizeof(desc), &desc);
		Assert(ok && "Failed to describe OpenGL pixel format");

		if (!SetPixelFormat(dc_w, format, &desc))
		{
			FatalError("Cannot set OpenGL selected pixel format!");
		}
	}
	//now create opengl context
	int attrib[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	#ifndef NDEBUG
		// ask for debug context for non "Release" builds
		// this is so we can enable debug callback
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
	#endif
		0
	};
	//https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
	//
	context = wglCreateContextAttribsARB(dc_w, NULL, attrib);
	//context = wglCreateContext(dc);
	if (!wglMakeCurrent(dc_w, context)) {
		std::cerr << "Failed to make context current\n";
	}
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_DEBUG_OUTPUT);
	glDisable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	loadFunctions();


	//start modern opengl needed stuff like shaders and vertex buffers
	glGenBuffers_g(1, &rotpoint_buffer);
	glBindBuffer_g(GL_ARRAY_BUFFER, rotpoint_buffer);
	glGenBuffers_g(1, &rot_buffer);
	glBindBuffer_g(GL_ARRAY_BUFFER, rot_buffer);
	glGenBuffers_g(1, &uv_buffer);
	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glGenBuffers_g(1, &vertex_buffer);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	//shader compiles
	//2d triangle shader and vertex array
	const char* vertex_shader_triangle2d = "#version 330 core\nlayout(location = 0) in vec2 aPos;\nuniform vec2 screen;\n"
		"void main(){\nvec2 p = aPos;\np /= screen;\np = (p * 2.0) - 1;\ngl_Position = vec4(p.x, -p.y, 0.0, 1.0);}\0";
	const char* fragment_shader_triangle2d = "#version 330 core\n"
		"out vec4 color;\n"
		"uniform vec4 set_color;\n"
		"void main(){\n"
		"color = set_color;\n"
		"}\0";
	shader_triangle2d = compileShader(vertex_shader_triangle2d, fragment_shader_triangle2d);
	glUseProgram_g(shader_triangle2d);
	glGenVertexArrays_g(1, &VAO_Triangle);
	glBindVertexArray_g(VAO_Triangle);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray_g(0);
	coloruniform_tri = glGetUniformLocation_g(shader_triangle2d, "set_color");
	GLuint screenuniform_tri = glGetUniformLocation_g(shader_triangle2d, "screen");
	glUniform2f_g(screenuniform_tri, (float)width, (float)height);

	//2d point shader and vertex array
	const char* vertex_shader_point = "#version 330 core\nlayout(location = 0) in vec2 pos;\nuniform vec2 screen;\nvoid main(){\n"
		"vec2 p = pos;\np /= screen;\np = (p * 2.0) - 1;\n"
		"gl_Position = vec4(p.x, p.y, 1.0, 1.0);\ngl_PointSize=1.0;}\0";
	const char* fragment_shader_point = "#version 330 core\nout vec4 color;\n"
		"uniform vec4 set_color;\n"
		"void main() {\n"
		"color = set_color;\n}\0";

	shader_point = compileShader(vertex_shader_point, fragment_shader_point);
	glUseProgram_g(shader_point);
	glGenVertexArrays_g(1, &VAO_Points);
	glBindVertexArray_g(VAO_Points);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray_g(0);
	coloruniform_point = glGetUniformLocation_g(shader_point, "set_color");
	screenuniform_tri = glGetUniformLocation_g(shader_point, "screen");
	glUniform2f_g(screenuniform_tri, (float)width, (float)height);

	const char* vertex_shader_line = "#version 330 core\nlayout(location = 0) in vec2 pos;\nuniform vec2 screen;\nvoid main(){\n"
		"vec2 p = pos;\np /= screen;\np = (p * 2.0) - 1;\n"
		"gl_Position = vec4(p.x, p.y, 1.0, 1.0);\n}\0";
	const char* fragment_shader_line = "#version 330 core\nout vec4 color;\n"
		"uniform vec4 set_color;\n"
		"void main(){\n"
		"color = set_color;\n}\0";

	shader_line = compileShader(vertex_shader_line, fragment_shader_line);
	glUseProgram_g(shader_line);
	glGenVertexArrays_g(1, &VAO_Line);
	glBindVertexArray_g(VAO_Line);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray_g(0);
	coloruniform_line = glGetUniformLocation_g(shader_line, "set_color");
	screenuniform_tri = glGetUniformLocation_g(shader_line, "screen");
	glUniform2f_g(screenuniform_tri, (float)width, (float)height);

	glBindVertexArray_g(0);

	const char* vertex_shader_img2 = "#version 330 core\nlayout (location = 0) in vec2 vec_pos;\nlayout (location = 1) in vec2 tex_point;\nout vec2 UV;\n"
		"uniform vec2 screen;\nvoid main() { \n"
		"vec2 p = vec_pos;\np /= screen;\np = (p * 2.0) - 1;\n"
		"gl_Position = vec4(p, 0.0, 1.0);\n"
		"UV = tex_point;\n"
		"}\0";
	const char* fragment_shader_img2 = "#version 330 core\nin vec2 UV;\nout vec4 color;\nuniform sampler2D image_tex;\nvoid main(){\n"
		"color = texture(image_tex, UV);\n}\0";	


	shader_img = compileShader(vertex_shader_img2, fragment_shader_img2);
	glGenVertexArrays_g(1, &VAO_Img);
	glUseProgram_g(shader_img);
	glBindVertexArray_g(VAO_Img);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray_g(0);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glEnableVertexAttribArray_g(1);
	glVertexAttribPointer_g(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	texuniform_img = glGetUniformLocation_g(shader_img, "image_tex");
	screenuniform_tri = glGetUniformLocation_g(shader_img, "screen");
	glUniform2f_g(screenuniform_tri, (float)width, (float)height);

	//conver this to screen space
	const char* vertex_shader_img_r = "#version 330 core\nlayout (location = 0) in vec2 vec_pos;\nlayout (location = 1) in vec2 tex_point;\n"
		"layout (location = 2) in float ang;\nlayout (location = 3) in vec2 rot_point;\nout vec2 UV;\nuniform vec2 screen;\nvoid main() {\n"
		//"vec2 p3 = vec2(vec_pos.x / screen.x, vec_pos.y / screen.y);\n"
		//"p3 = vec2((p3.x * 2.0) - 1, (p3.y * 2.0) - 1);\n"
		//"vec2 p4 = vec2(rot_point.x / screen.x, rot_point.y / screen.y);\n"
		//"p4 = vec2((p4.x * 2.0) - 1, (p4.y * 2.0) - 1);\n"
		//"vec2 p1 = vec2(p3.x - p4.x, p3.y - p4.y);\n"
		"vec2 p1 = vec2(vec_pos.x - rot_point.x, vec_pos.y - rot_point.y);\n"
		"vec2 p = vec2(p1.x*cos(ang)-p1.y*sin(ang), p1.y*cos(ang) + p1.x*sin(ang));\n"
		//"p = vec2(p.x + p4.x, p.y + p4.y);\n"
		"p = vec2(p.x + rot_point.x, p.y + rot_point.y);\n"
		//"p /= screen;\n"
		//"p = (p * 2.0) - 1;\n"
		"gl_Position = vec4(p, 0.0, 1.0);\n"
		"UV = tex_point;\n"
		"}\0";
	const char* fragment_shader_img_r = "#version 330 core\nin vec2 UV;\nout vec4 color;\nuniform sampler2D image_tex;\nvoid main(){\n"
		"color = texture(image_tex, UV);\n}\0";
	shader_imgr = compileShader(vertex_shader_img_r, fragment_shader_img_r);
	glGenVertexArrays_g(1, &VAO_Imgr);
	glUseProgram_g(shader_img);
	glBindVertexArray_g(VAO_Imgr);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray_g(0);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer_g(GL_ARRAY_BUFFER, uv_buffer);
	glEnableVertexAttribArray_g(1);
	glVertexAttribPointer_g(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer_g(GL_ARRAY_BUFFER, rot_buffer);
	glEnableVertexAttribArray_g(2);
	glVertexAttribPointer_g(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer_g(GL_ARRAY_BUFFER, rotpoint_buffer);
	glEnableVertexAttribArray_g(3);
	glVertexAttribPointer_g(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	texuniform_imgr = glGetUniformLocation_g(shader_imgr, "image_tex");
	screenuniform_tri = glGetUniformLocation_g(shader_imgr, "screen");
	glUniform2f_g(screenuniform_tri, (float)width, (float)height);


	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindVertexArray_g(0);
	//speed up push_backs
	buffer_2d.reserve(1000);
	buffer_uv.reserve(1000);
	rotations.reserve(1000);
	buffer_img.reserve(1000);
	rot_points.reserve(1000);
	img_buffer.reserve(1000);


	ShowWindow(wind->getHwnd(), SW_SHOW);
}


//shader functions

GLuint EngineNewGL::compileShader(const char* vertex_file, const char* fragment_file) {
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
	return ProgramID;
}