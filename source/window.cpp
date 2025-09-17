#include "backend.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_PAINT:

		break;
	case WM_SIZE:
		{
			uint32_t width = LOWORD(lparam);
			uint32_t height = HIWORD(lparam);
			glViewport(0, 0, width, height);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
#endif

g_window::~g_window() {
	//has to be same class name
	#if defined(_WIN32)
	UnregisterClass("GENG", r_display);
	#endif
	#if defined(__unix__)
    XDestroyWindow(r_display, m_hwnd);
	#endif
}

bool g_window::ProcessMessage() {
	#if defined(_WIN32)
	MSG msg = {};

	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	#endif
	#if defined(__unix__)
	XEvent event;
        if (XPending(r_display)) {
            XNextEvent(r_display, &event);
			if (event.type == DestroyNotify) {
				return false;
			}

		}
	#endif
	return true;
}

bool g_window::updateWindow() {
	#if defined(_WIN32)
	return UpdateWindow(getRawWindow());
	#endif
	#if defined(__unix__)
	return XMapWindow(r_display, m_hwnd);
	#endif
}


bool g_window::swapBuffers() {
	#if defined(_WIN32)
	return SwapBuffers(GetDC(getRawWindow()));
	#endif
	#if defined(__unix__)
	glXSwapBuffers(r_display, m_hwnd);
	return true;
	#endif
}
#if defined(_WIN32)
g_window::g_window(const char* title, RAW_DISPLAY r_display, int h, int w, int x, int y, bool fullscreen)
	: r_display(GetModuleHandle(nullptr))
{
	WNDCLASS wnd = {};
	wnd.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wnd.lpszClassName = "GENG";
	wnd.hInstance = r_display;
	wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = NULL;
	wnd.lpszMenuName = NULL;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.lpfnWndProc = WindowProc;

	RegisterClass(&wnd);

	DWORD windStyle = WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU;
	height = h;
	width = w;

	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	AdjustWindowRect(&rect, windStyle, false);

	m_hwnd = CreateWindowEx(0, wnd.lpszClassName,
		title,
		windStyle,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		r_display,
		NULL
	);
}
#endif

#if defined(__unix__)
g_window::g_window(const char* title, RAW_DISPLAY display, int h, int w, int x, int y, bool fullscreen) {
    if (!display) {
        std::cout << "Unable to open X display\n";
        exit(1);
    }
	this->r_display = display;
	height = h;
	width = w;
    Window root = DefaultRootWindow(display);
	// Choose framebuffer config
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
	GLX_DOUBLEBUFFER    , True,
	None
	};
	int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(r_display, DefaultScreen(r_display), visual_attribs, &fbcount);
    if (!fbc) {
        fprintf(stderr, "Failed to get framebuffer config\n");
        return;
    }

    GLXFBConfig fbconfig = fbc[0];
    XVisualInfo* vi = glXGetVisualFromFBConfig(r_display, fbconfig);

    // Create window
    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(r_display, RootWindow(r_display, vi->screen), vi->visual, AllocNone);
    swa.event_mask = ExposureMask | KeyPressMask;

    m_hwnd = XCreateWindow(r_display, root, 0, 0, 800, 600, 0, vi->depth,
                        InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	if (fullscreen) {
		XWindowAttributes window_attributes;
		XGetWindowAttributes(r_display, root, &window_attributes);
		XResizeWindow(r_display, m_hwnd, window_attributes.width, window_attributes.height);
		Atom wm_state = XInternAtom(r_display, "_NET_WM_STATE" ,False);
		Atom wm_fullscreen = XInternAtom(r_display, "_NET_WM_STATE_FULLSCREEN", False);
		XChangeProperty(r_display, m_hwnd, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wm_fullscreen, 1);
	}
	XMapWindow(r_display, m_hwnd);
	XStoreName(r_display, m_hwnd, title);
	XFree(vi);
	XFree(fbc);

}
#endif

int g_window::getWidth() {
	return width;
}
int g_window::getHeight() {
	return height;
}
RAW_WINDOW g_window::getRawWindow() {
	return m_hwnd;
}
RAW_DISPLAY g_window::getRawDisplay() {
	return r_display;
}


void g_window::setWindowFullscreen() {
	#if defined(_WIN32)
	DWORD dwStyle = GetWindowLong(m_hwnd, GWL_STYLE);
	MONITORINFO mi = { sizeof(mi) };
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST), &mi);
	SetWindowLongPtr(m_hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
	SetWindowPos(m_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	#endif
	#if defined(__unix__)
	 	// need to use xrandr lol!
		int32_t screen = DefaultScreen(r_display);
		Window root = RootWindow(r_display, screen);
		int32_t screen_width = DisplayWidth(r_display, screen);
		int32_t screen_height = DisplayHeight(r_display, screen);
		XResizeWindow(r_display, m_hwnd, screen_width, screen_height);
		std::cout << screen_width << " : " << screen_height << "\n";
		Atom wm_state = XInternAtom(r_display, "_NET_WM_STATE" ,False);
		Atom fullscreen = XInternAtom(r_display, "_NET_WM_STATE_FULLSCREEN", False);
		XChangeProperty(r_display, m_hwnd, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&fullscreen, 1);
		XMoveWindow(r_display, m_hwnd, 0, 0);
		XMapWindow(r_display, m_hwnd);
	#endif
}