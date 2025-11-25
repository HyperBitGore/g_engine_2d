#include "g_engine_2d.hpp"
#include "rendering/font_renderer.hpp"
#include "rendering/image_renderer.hpp"
#include "rendering/primitive_renderer.hpp"
#include "util/shader.hpp"
#include <memory>

#if defined (__unix__)
int myXIOErrorHandler(Display *dpy) {
    fprintf(stderr, "X server connection lost. Cleaning up.\n");
    // You could save state, attempt recovery, etc.
    exit(EXIT_FAILURE); // Avoid calling exit(), which may trigger undefined behavior in some cases
}
#endif

//https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
EngineNewGL::EngineNewGL(const char* window_name, int width, int height, uint8_t component_mask) {
	#if defined(_WIN32)
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
	wind = new g_window(window_name, nullptr, height, width, 300, 300);
	in = new Input(wind->getRawDisplay(), wind->getRawWindow());
	HDC dc_w = GetDC(wind->getRawWindow());
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
	#endif
	#if defined(__unix__)
	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	display = XOpenDisplay(NULL);
    if (!display) FatalError("Cannot open X display");

    static int visual_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        // Uncomment for multisampling:
        // GLX_SAMPLE_BUFFERS  , 1,
        // GLX_SAMPLES         , 4,
        None
    };
	int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
    if (!fbc) {
        FatalError("Failed to get framebuffer config");
    }

	GLXFBConfig fbconfig = fbc[0];

	wind = new g_window(window_name, display, height, width, 300, 300);
	in = new Input(wind->getRawDisplay(), wind->getRawWindow());

    if (!fbc || fbcount == 0) FatalError("Failed to get FBConfig");
	// Load context creation function
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
	(glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    ctx = glXCreateContextAttribsARB(display, fbconfig, 0, True, context_attribs);
    if (!ctx) {
        FatalError("failed to create gl context");
    }

    XFree(fbc);

	if (!glXMakeCurrent(display, wind->getRawWindow(), ctx)) {
        FatalError("Failed to make context current");
    }
	XSetIOErrorHandler(myXIOErrorHandler);
	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = 
    (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
	if (glXSwapIntervalEXT) {
    	glXSwapIntervalEXT(display, glXGetCurrentDrawable(), 0); // 0 = disable vsync
	} else {
		typedef int (*glXSwapIntervalMESAFunc)(unsigned int);
		auto glXSwapIntervalMESA = (glXSwapIntervalMESAFunc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
		if (glXSwapIntervalMESA) {
			glXSwapIntervalMESA(0);
		} else {
			typedef int (*glXSwapIntervalSGIFunc)(int);
			auto glXSwapIntervalSGI = (glXSwapIntervalSGIFunc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
			if (glXSwapIntervalSGI) {
				glXSwapIntervalSGI(0);
			}
		}
	}
	#endif
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_DEBUG_OUTPUT);
	glDisable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	int test[4];
	glGetIntegerv(GL_VIEWPORT, test);
	std::cout << test[0] << " : " << test[1] << "\n";
	//glViewport(0, 0, width, height);

	loadFunctions();
	glViewport(0, 0, width, height);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units); //getting the texture units useable at a time on this machine
	std::cout << "Texture Units on this machine: " << texture_units << "\n";
	//start modern opengl needed stuff like shaders and vertex buffers
	if (component_mask & PRIMITIVE_COMPONENT) {
		this->prim_r = std::make_unique<PrimitiveRenderer>(width, height);
	}
	if (component_mask & IMAGE_COMPONENT) {
		this->img_r = std::make_unique<imagerenderer>(width, height);
	}
	if (component_mask & GRAYSCALE_COMPONENT) {
		this->gray_r = std::make_unique<grayscalerenderer>(width, height);
	}
	if (component_mask & FONT_COMPONENT) {
		this->font_r = std::make_unique<gore::FontRenderer>(width, height);
	}
	#if defined(_WIN32)
	ShowWindow(wind->getRawWindow(), SW_SHOW);
	#endif
}
