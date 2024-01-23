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


	ShowWindow(wind->getHwnd(), SW_SHOW);
}
