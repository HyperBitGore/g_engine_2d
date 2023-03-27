#include "g_engine_2d.h"



//2d drawing functions

void EngineNewGL::draw2dFrame() {
	float vertices[] = {
		// first triangle
		 0.5f,  0.5f, 0.0f,  // top right
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f,  0.5f, 0.0f,  // top left 
		// second triangle
		 0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left
	};
	glUseProgram_g(shader_2d);
	glBindVertexArray_g(VAO2D);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	//glBufferData_g(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData_g(GL_ARRAY_BUFFER, buffer_2d.size() * sizeof(vec3), &buffer_2d[0], GL_STATIC_DRAW);
	glVertexAttribPointer_g(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray_g(0);
	glDrawArrays(GL_TRIANGLES, 0, buffer_2d.size());
	buffer_2d.clear();
	glDisableVertexAttribArray_g(0);
}


void EngineNewGL::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	buffer_2d.push_back({ x1, y1, 1.0f });
	buffer_2d.push_back({ x2, y2, 1.0f });
	buffer_2d.push_back({ x3, y3, 1.0f });

}



//utility type functions


bool EngineNewGL::updateWindow() {
	UpdateWindow(wind->getHwnd());
	if (!wind->ProcessMessage()) {
		std::cout << "Closing window\n";
		delete wind;
		return false;
	}
	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderFund();
	

	if (!SwapBuffers(dc_w))
	{
		FatalError("Failed to swap OpenGL buffers!");
	}
	return true;
}


//https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
EngineNewGL::EngineNewGL(LPCWSTR window_name, int width, int height) {
	PIXELFORMATDESCRIPTOR windowPixelFormatDesc = { 0 };
	windowPixelFormatDesc.nSize = sizeof(windowPixelFormatDesc);
	windowPixelFormatDesc.nVersion = 1;
	windowPixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
	windowPixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	windowPixelFormatDesc.cColorBits = 32;
	windowPixelFormatDesc.cAlphaBits = 8;
	windowPixelFormatDesc.iLayerType = PFD_MAIN_PLANE;
	windowPixelFormatDesc.cDepthBits = 24;
	windowPixelFormatDesc.cStencilBits = 8;

	//function pointers
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

	//create window
	wind = new Window(window_name, L"ENG1", width, height, 300, 300);
	in = new Input();
	//getting device context
	dc_w = GetDC(wind->getHwnd());
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
			desc.cColorBits = 24,
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
	dc_w = GetDC(wind->getHwnd());

	// set pixel format for OpenGL context
	{
		int attrib[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
			WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB,     24,
			WGL_DEPTH_BITS_ARB,     24,
			WGL_STENCIL_BITS_ARB,   8,

			// uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
			//WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

			// uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
			//WGL_SAMPLE_BUFFERS_ARB, 1,
			//WGL_SAMPLES_ARB,        4, // 4x MSAA

			0,
		};

		int format;
		UINT formats;
		if (!wglChoosePixelFormatARB(dc_w, attrib, NULL, 1, &format, &formats) || formats == 0)
		{
			FatalError("OpenGL does not support required pixel format!");
		}

		PIXELFORMATDESCRIPTOR desc;
		desc.nSize = sizeof(desc);
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
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	#ifndef NDEBUG
		// ask for debug context for non "Release" builds
		// this is so we can enable debug callback
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
	#endif
		0,
	};
	context = wglCreateContextAttribsARB(dc_w, NULL, attrib);
	//context = wglCreateContext(dc);
	if (!wglMakeCurrent(dc_w, context)) {
		std::cerr << "Failed to make context current\n";
	}
	//glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);
	/*GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed, something is seriously wrong. 
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));*/
	//glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	loadFunctions();


	//start modern opengl needed stuff like shaders and vertex buffers
	glGenVertexArrays_g(1, &VAO2D);
	glBindVertexArray_g(VAO2D);

	glGenBuffers_g(1, &vertex_buffer);
	glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
	//shader compiles
	const char* vertex_shader_2d = "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid main(){\ngl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);}\0";
	const char* fragment_shader_2d = "#version 330 core\n"
		"out vec3 color;\n"
		"void main(){\n"
		"color = vec3(1.0f, 0.0f, 0.0f);\n"
		"}\0";
	shader_2d = compileShader(vertex_shader_2d, fragment_shader_2d);


	ShowWindow(wind->getHwnd(), SW_SHOW);
}




GLuint EngineNewGL::compileShader(const char* vertex, const char* fragment) {
	unsigned int vertexShader;
	vertexShader = glCreateShader_g(GL_VERTEX_SHADER);
	
	glShaderSource_g(vertexShader, 1, &vertex, NULL);
	glCompileShader_g(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv_g(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog_g(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader_g(GL_FRAGMENT_SHADER);
	glShaderSource_g(fragmentShader, 1, &fragment, NULL);
	glCompileShader_g(fragmentShader);

	glGetShaderiv_g(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog_g(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	//actually creating program
	GLuint program;
	program = glCreateProgram_g();
	glAttachShader_g(program, vertexShader);
	glAttachShader_g(program, fragmentShader);
	glLinkProgram_g(program);
	//check error
	glGetProgramiv_g(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog_g(program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::LINK::FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader_g(vertexShader);
	glDeleteShader_g(fragmentShader);
	return program;
}