#include "backend.hpp"


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

g_window::~g_window() {
	//has to be same class name
	UnregisterClass(class_name, m_hinstance);
}

bool g_window::ProcessMessage() {
	MSG msg = {};

	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

bool g_window::updateWindow() {
	return UpdateWindow(getRawWindow());
}


bool g_window::swapBuffers() {
	return SwapBuffers(GetDC(getRawWindow()));
}

g_window::g_window(const char* title, const char* CLASS_NAME, int h, int w, int x, int y)
	: m_hinstance(GetModuleHandle(nullptr))
{
	class_name = CLASS_NAME;
	WNDCLASS wnd = {};
	wnd.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wnd.lpszClassName = CLASS_NAME;
	wnd.hInstance = m_hinstance;
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
		m_hinstance,
		NULL
	);
}


int g_window::getWidth() {
	return width;
}
int g_window::getHeight() {
	return height;
}
RAW_WINDOW g_window::getRawWindow() {
	return m_hwnd;
}