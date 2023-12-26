#include "g_engine_2d.h"

int indexs_hash(GLint n) {
	return n % 48;
}


//https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
EngineNewGL::EngineNewGL(LPCSTR window_name, int width, int height) {
	//indexs.setHashFunction(indexs_hash);

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
	wind = new Window(window_name, "ENG1", width, height, 300, 300);
	in = new Input();
	HDC dc_w = GetDC(wind->getHwnd());
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
	HGLRC context = wglCreateContextAttribsARB(dc_w, NULL, attrib);
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
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units); //getting the texture units useable at a time on this machine
	std::cout << "Texture Units on this machine: " << texture_units << "\n";

	//start modern opengl needed stuff like shaders and vertex buffers
	glGenBuffers_g(1, &img_buffer);
	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	//making ssbo
	glGenBuffers_g(1, &texture_buffer);
	glBindBuffer_g(GL_SHADER_STORAGE_BUFFER, texture_buffer);
	

	//shader compiles

	glBindVertexArray_g(0);

	const char* vertex_shader_img2 = "#version 430 core\nlayout (location = 0) in vec2 vec_pos;\nlayout (location = 1) in vec2 tex_point;\n"
		"out vec2 UV; \n"
		"uniform vec2 screen;\nvoid main() { \n"
		"vec2 p = vec_pos;\np /= screen;\np = (p * 2.0) - 1;\n"
		"gl_Position = vec4(p, 0.0, 1.0);\n"
		"UV = tex_point;\n"
		"}\0";
	std::string fragment_shader_img2 = "#version 430 core\nin vec2 UV;\nout vec4 color;\nuniform sampler2D image_tex;\n"
		"void main() { \n"
		"color = texture(image_tex, UV);\n}\0";
	

	shader_img = compileShader(vertex_shader_img2, fragment_shader_img2.c_str());
	glGenVertexArrays_g(1, &VAO_Img);
	glUseProgram_g(shader_img);
	glBindVertexArray_g(VAO_Img);
	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	glEnableVertexAttribArray_g(0);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(img_vertex), (void*)0);
	glEnableVertexAttribArray_g(1);
	glVertexAttribPointer_g(1, 2, GL_FLOAT, GL_FALSE, sizeof(img_vertex), (void*)12);

	texuniform_img = glGetUniformLocation_g(shader_img, "image_tex");

	//conver this to screen space
	const char* vertex_shader_img_r = "#version 430 core\nlayout (location = 0) in vec2 vec_pos;\nlayout (location = 1) in vec2 tex_point;\n"
		"layout (location = 2) in float ang;\nlayout (location = 3) in vec2 rot_point;\nout vec2 UV;\n"
		"uniform vec2 screen;\n"
		"void main() { \n"
		"vec2 test = screen;\n"
		"vec2 rot_p = rot_point;\nrot_p = rot_p / test;\nrot_p = (rot_p * 2.0) - 1;\n"
		"vec2 p3 = vec_pos;\np3 = p3 / test;\np3 = (p3 * 2.0) - 1;\n"
		"vec2 p1 = vec2(p3.x - rot_p.x, p3.y - rot_p.y);\n"
		"vec2 p = vec2(p1.x*cos(ang)-p1.y*sin(ang), p1.y*cos(ang) + p1.x*sin(ang));\n"
		"p = vec2(p.x + rot_p.x, p.y + rot_p.y);\n"
		"gl_Position = vec4(p, 0.0, 1.0);\n"
		"UV = tex_point;\n"
		"}\0";
	const char* fragment_shader_img_r = "#version 430 core\nin vec2 UV;\nout vec4 color;\nuniform sampler2D image_tex;\n"
		""
		"void main() { \n"
		"color = texture(image_tex, UV);\n}\0";
	shader_imgr = compileShader(vertex_shader_img_r, fragment_shader_img_r);
	glGenVertexArrays_g(1, &VAO_Imgr);
	glUseProgram_g(shader_img);
	glBindVertexArray_g(VAO_Imgr);
	
	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	glEnableVertexAttribArray_g(0);
	glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(img_vertex), (void*)0);
	
	
	glEnableVertexAttribArray_g(1);
	glVertexAttribPointer_g(1, 2, GL_FLOAT, GL_FALSE, sizeof(img_vertex), (void*)12);
	
	
	glEnableVertexAttribArray_g(2);
	glVertexAttribPointer_g(2, 1, GL_FLOAT, GL_FALSE, sizeof(img_vertex), (void*)20);
	
	
	glEnableVertexAttribArray_g(3);
	glVertexAttribPointer_g(3, 2, GL_FLOAT, GL_FALSE, sizeof(img_vertex), (void*)24);
	
	texuniform_imgr = glGetUniformLocation_g(shader_imgr, "image_tex");


	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindVertexArray_g(0);
	//speed up push_backs
	img_vertexs.reserve(1000);


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