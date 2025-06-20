#include "backend.hpp"
#include <cstdlib>
#include <iostream>

#if defined(_WIN32)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_PAINT:

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
	UnregisterClass(class_name, m_hinstance);
	#endif
	#if defined(__unix__)
    XCloseDisplay(display);
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
        if (XPending(display)) {
            XNextEvent(display, &event);
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
	return XMapWindow(display, m_hwnd);
	#endif
}


bool g_window::swapBuffers() {
	#if defined(_WIN32)
	return SwapBuffers(GetDC(getRawWindow()));
	#endif
	#if defined(__unix__)
	glXSwapBuffers(display, m_hwnd);
	#endif
}
#if defined(_WIN32)
g_window::g_window(const char* title, const char* CLASS_NAME, int h, int w, int x, int y)
	: display(GetModuleHandle(nullptr))
{
	class_name = CLASS_NAME;
	WNDCLASS wnd = {};
	wnd.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wnd.lpszClassName = CLASS_NAME;
	wnd.hInstance = display;
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

	m_hwnd = CreateWindowEx(0, CLASS_NAME,
		title,
		windStyle,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		display,
		NULL
	);
}
#endif

#if defined(__unix__)
g_window::g_window(const char* title, const char* CLASS_NAME, int h, int w, int x, int y) {
	display = XOpenDisplay(NULL);
    if (!display) {
        std::cout << "Unable to open X display\n";
        exit(1);
    }
    Window root = DefaultRootWindow(display);
    XVisualInfo *vi = glXChooseVisual(display, 0, NULL);
    if (!vi) {
        std::cout << "No appropriate visual found\n";
        exit(1);
    }

    Colormap cmap = XCreateColormap(display, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    m_hwnd = XCreateWindow(display, root, 0, 0, 800, 600, 0, vi->depth,
                        InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	XMapWindow(display, m_hwnd);
	XStoreName(display, m_hwnd, title);
					

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
	return display;
}