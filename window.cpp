#include "g_engine_2d.h"


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

bool Window::ProcessMessage() {
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



Window::Window(LPCWSTR title, LPCWSTR CLASS_NAME, int h, int w, int x, int y)
	: m_hinstance(GetModuleHandle(nullptr))
{
	class_name = CLASS_NAME;
	WNDCLASS wnd = {};
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd.lpszClassName = CLASS_NAME;
	wnd.hInstance = m_hinstance;
	wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wnd.hCursor = LoadIcon(NULL, IDC_ARROW);
	wnd.hbrBackground = 0;
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