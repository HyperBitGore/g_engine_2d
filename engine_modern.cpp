#include "g_engine_2d.h"




void EngineNewGL::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	
}



bool EngineNewGL::updateWindow() {
	UpdateWindow(wind->getHwnd());
	if (!wind->ProcessMessage()) {
		std::cout << "Closing window\n";
		delete wind;
		return false;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderFund();
	SwapBuffers(dc_w);
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
	loadFunctions();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	
	//start modern opengl needed stuff like shaders and vertex buffers
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//shader compiles
	const char* vertex_shader_2d = "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid main(){\ngl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);}\0";
	const char* fragment_shader_2d = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main(){\n"
		"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";
	shader_2d = compileShader(vertex_shader_2d, fragment_shader_2d);

	ShowWindow(wind->getHwnd(), SW_SHOW);
}




GLuint EngineNewGL::compileShader(const char* vertex, const char* fragment) {
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	
	glShaderSource(vertexShader, 1, &vertex, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	//actually creating program
	GLuint program;
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	//check error
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::LINK::FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}