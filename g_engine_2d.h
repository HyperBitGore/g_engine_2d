#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <gl/GL.h>
#include <functional>
#include <math.h>
#include "lodepng.h"
#define M_PI 3.14159265358979323846
#define pack_rgba(r,g,b,a) (uint32_t)(r<<24|g<<16|b<<8|a)
#define unpack_r(col) (uint8_t)((col>>24)&0xff)
#define unpack_g(col) (uint8_t)((col>>16)&0xff)
#define unpack_b(col) (uint8_t)((col >> 8)&0xff)
#define unpack_a(col) (uint8_t)(col&0xff)




//bmp file loading
//draw text
//update to newer version of gl
//add shader support
//add 3d support


struct g_img {
	unsigned char* data;
	GLuint tex;
	unsigned int w;
	unsigned int h;
};

typedef g_img* IMG;

LRESULT	CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//HDC hdc = GetDC(hwnd);
	//PAINTSTRUCT ps;
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

//added customizability and clean up
class Window {
private:
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	LPCWSTR class_name;
	int height;
	int width;
public:
	Window(LPCWSTR title, LPCWSTR CLASS_NAME, int h, int w, int x, int y) 
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
	Window(const Window&) = delete;
	Window& operator =(const Window&) = delete;
	~Window() {
		//has to be same class name
		UnregisterClass(class_name, m_hinstance);
	}
	bool ProcessMessage() {
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
	int getWidth() {
		return width;
	}
	int getHeight() {
		return height;
	}

	HWND getHwnd() {
		return m_hwnd;
	}
};



class Input {
private:
	HKL layout;
	char keys[256];
	char last_state[256];
public:
	Input() {
		layout = LoadKeyboardLayout(L"00000409", KLF_ACTIVATE);
	}
	Input(const Input&) = delete;
	Input& operator =(const Input&) = delete;
	void setLastState() {
		for (int i = 0; i < 256; i++) {
			last_state[i] = keys[i];
		}
	}
	//input functions
	//probably switch to GetKeyboardState, so easier to use getKeyReleased
	void getState() {
		setLastState();
		bool fail = GetKeyboardState((PBYTE)keys);
	}


	bool GetKeyDown(char key) {
		getState();
		short t = key;
		if (key >= 97 && key <= 122) {
			t = VkKeyScanEx(key, layout);
		}
		if ((keys[t] >> 15) == -1) {
			return true;
		}
		return false;
	}
	bool getKeyReleased(char key) {
		short t = key;
		if (key >= 97 && key <= 122) {
			t = VkKeyScanEx(key, layout);
		}
		if ((last_state[t]>>15) != (keys[t]>>15) && (keys[t]>>15) != -1) {
			getState();
			return true;
		}
		getState();
		return false;
	}
};

class ImageLoader {
private:
	static void readBMPPixels32(IMG f, std::string str, size_t offset, size_t raw_size) {
		char* t = (char*)str.c_str();
		char* fo = (char*)f->data;
		t += offset;
		size_t row_size = f->w;
		if (f->w % 4 != 0) {
			row_size += (f->w % 4);
		}
		size_t j = 0;
		int c = 0;
		char alpha_val = 0;
		char r_val;
		char g_val;
		char b_val;
		size_t r_spot = j;
		for (size_t i = 0; i < raw_size; i++, j++) {
			switch (c) {
			case 0:
				r_spot = j;
				alpha_val = *(t + i);
				c++;
				break;
			case 1:
				r_val = *(t + i);
				c++;
				break;
			case 2:
				g_val = *(t + i);
				c++;
				break;
			case 3:
				b_val = *(t + i);
				f->data[r_spot] = r_val;
				f->data[r_spot + 1] = g_val;
				f->data[r_spot + 2] = b_val;
				f->data[r_spot + 3] = alpha_val;
				c = 0;
				break;
			}
		}
	}
	//fix color being off, ie read color table
	static void readBMPPixels24(IMG f, std::string str, size_t offset, size_t raw_size) {
		char* t = (char*)str.c_str();
		char* fo = (char*)f->data;
		t += offset;
		std::memcpy(f->data, t, raw_size);
	}
	static void parseBMPData(IMG f, std::string str, size_t offset, unsigned short bitsperpixel, size_t size) {
		//add reading color table later

		//read pixel data
		switch (bitsperpixel) {
		case 1:
			break;
		case 2:
			break;
		case 4:
			break;
		case 8:
			break;
		case 16:
			break;
		case 24:
			readBMPPixels24(f, str, offset, size);
			glGenTextures(1, &f->tex);
			glBindTexture(GL_TEXTURE_2D, f->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, f->w, f->h, 0, GL_RGB, GL_UNSIGNED_BYTE, f->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case 32:
			readBMPPixels32(f, str, offset, size);
			glGenTextures(1, &f->tex);
			glBindTexture(GL_TEXTURE_2D, f->tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, f->w, f->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		}
	}
public:
	//https://docs.fileformat.com/image/bmp/
	//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	//https://en.wikipedia.org/wiki/BMP_file_format
	static IMG loadBMP(std::string file) {
		std::ifstream f;
		f.open(file);
		std::ostringstream stram;
		stram << f.rdbuf();
		std::string t = stram.str();
		//declaring all the attribute variables
		size_t f_size, offset, width, height, raw_size, num_cols, num_imcols;
		unsigned short bitsperpixel;
		int compression, hor_res, ver_res;
		char* fo = (char*)t.c_str();
		//read first two bytes and make sure they are BM
		if (*fo != 'B' || *(fo + 1) != 'M') {
			return nullptr;
		}
		//now read file size
		fo += 2;
		int* fi = (int*)fo;
		f_size = *fi;
		fo += 8;
		//get offset for image bytes
		fi = (int*)fo;
		offset = *fi;
		//read info header, assuming using info header, add support for older versions later
		fi += 2;
		width = *fi;
		fi++;
		height = *fi;
		short* fs = (short*)fi;
		fs += 3;
		bitsperpixel = *fs;
		fs++;
		fi = (int*)fs;
		compression = *fi;
		fi++;
		raw_size = *fi;
		fi++;
		hor_res = *fi;
		fi++;
		ver_res = *fi;
		fi++;
		num_cols = *fi;
		fi++;
		num_imcols = *fi;
		fi++;
		//read actual file
		IMG out = new g_img;
		out->w = width;
		out->h = height;
		out->data = (unsigned char*)std::malloc(raw_size);
		parseBMPData(out, t, 54, bitsperpixel, raw_size);
		return out;
	}
	static IMG loadPNG(std::string file, unsigned int w, unsigned int h) {
		IMG f = new g_img;
		unsigned error = lodepng_decode32_file((&(f->data)), &w, &h, file.c_str());
		if (error) {
			std::cout << lodepng_error_text(error) << std::endl;
		}
		f->w = w;
		f->h = h;
		glGenTextures(1, &f->tex);
		glBindTexture(GL_TEXTURE_2D, f->tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, f->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		return f;
	}
};



class Engine {
private:
	Window* wind;
	Input* in;
	HDC dc;
	HGLRC context;
	std::function<void()> renderFund;
public:
	Engine();
	Engine(LPCWSTR window_name, int width, int height, int x, int y) {
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

		//create window
		wind = new Window(window_name, L"ENG1", width, height, x, y);
		in = new Input();
		//getting device context
		dc = GetDC(wind->getHwnd());
		//choosing pixel format
		int pixelFormat = ChoosePixelFormat(dc, &windowPixelFormatDesc);
		//set pixel format
		SetPixelFormat(dc, pixelFormat, &windowPixelFormatDesc);
		//now create opengl context
		context = wglCreateContext(dc);
		if (!wglMakeCurrent(dc, context)) {
			std::cerr << "Failed to make context current\n";
		}
		//glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glEnable(GL_TEXTURE_2D);
		ShowWindow(wind->getHwnd(), SW_SHOW);
		
	}
	~Engine() {

	}
	void setRenderFunction(std::function<void()> func) {
		renderFund = func;
	}
	//Create a modern opengl context
	//https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
	bool updateWindow() {
		UpdateWindow(wind->getHwnd());
		if (!wind->ProcessMessage()) {
			std::cout << "Closing window\n";
			delete wind;
			return false;
		}
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderFund();
		SwapBuffers(dc);
		return true;
	}
	//primitive rendering
	void drawTriangle(float x, float y, float size) {
		glBegin(GL_TRIANGLES);
		glVertex2f(x - size, y - size);
		glVertex2f(x, y);
		glVertex2f(x + size, y - size);
		glEnd();
	}
	void drawCircle(float x, float y, float r) {
		float x1, y1;
		glBegin(GL_POINTS);
		for (float ang = 0; ang < 360; ang += 1.0f) {
			x1 = r * cos(ang * M_PI / 180) + x;
			y1 = r * sin(ang * M_PI / 180) + y;
			glVertex2f(x1, y1);
		}
		glEnd();
	}
	void drawPoint(float x, float y) {
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();
	}
	void drawRectangle(float x, float y, float w, float h) {
		glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x, y - h);
		glVertex2f(x + w, y - h);
		glVertex2f(x + w, y);
		glEnd();
	}
	void drawLine(float x1, float y1, float x2, float y2) {
		glBegin(GL_POINTS);
		float dx = x2 - x1;
		float dy = y2 - y1;
		//draw points while we travel with angle to second point
		if (x1 > x2) {
			for (float x = x1; x > x2; x += -0.001f) {
				float y = y1 + dy * (x - x1) / dx;
				glVertex2f(x, y);
			}
		}
		else {
			for (float x = x1; x < x2; x += 0.001f) {
				float y = y1 + dy * (x - x1) / dx;
				glVertex2f(x, y);
			}
		}
		glEnd();
	}


	//file functions
	void loadShader(std::string file) {

	}

	//image functions
	void renderImg(IMG img, float x, float y, int w, int h) {
		float r_w = (float(w) / wind->getWidth());
		float r_h = (float(h) / wind->getHeight());
		glBindTexture(GL_TEXTURE_2D, img->tex);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(1, 0);
		glVertex2f(x + r_w, y);
		glTexCoord2f(1, 1);
		glVertex2f(x + r_w, y - r_h);
		glTexCoord2f(0, 1);
		glVertex2f(x, y - r_h);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//rotates counter clockwise around top left point
	void renderImgRotated(IMG img, float x, float y, int w, int h, int ang) {
		float r_ang = float(ang) * M_PI / 180.0f;
		float r_cos = cosf(r_ang);
		float r_sin = sinf(r_ang);
		float r_x = (x*r_cos) - (y*r_sin);
		float r_y = (x*r_sin) + (y*r_cos);
		float r_w = (float(w) / wind->getWidth());
		float r_h = (float(h) / wind->getHeight());
		glBindTexture(GL_TEXTURE_2D, img->tex);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(((x * r_cos) - (y * r_sin)), ((x * r_sin) + (y * r_cos)));
		glTexCoord2f(1, 0);
		glVertex2f(((x + r_w) * r_cos) - (y * r_sin), (((x+r_w) * r_sin) + (y * r_cos)));
		glTexCoord2f(1, 1);
		glVertex2f(((x + r_w) * r_cos) - ((y-r_h) * r_sin), (((x+r_w) * r_sin) + ((y-r_h) * r_cos)));
		glTexCoord2f(0, 1);
		glVertex2f(((x * r_cos) - ((y-r_h) * r_sin)), ((x * r_sin) + ((y - r_h) * r_cos)));
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		size_t row = y * (img->w*4);
		size_t col = x * 4;
		img->data[row + col] = r;
		img->data[row + col + 1] = g;
		img->data[row + col + 2] = b;
		img->data[row + col + 3] = a;
	}
	void setPixel(IMG img, int x, int y, uint32_t color) {
		size_t row = y * (img->w * 4);
		size_t col = x * 4;
		img->data[row + col] = unpack_r(color);
		img->data[row + col + 1] = unpack_g(color);
		img->data[row + col + 2] = unpack_b(color);
		img->data[row + col + 3] = unpack_a(color);
	}
	uint32_t getPixel(IMG img, int x, int y) {
		size_t row = y * (img->w * 4);
		size_t col = x * 4;
		return pack_rgba(img->data[row + col], img->data[row + col + 1], img->data[row + col + 2], img->data[row + col + 3]);
	}
	//run after you've done all the editing of data you want to
	void updateIMG(IMG img) {
		glBindTexture(GL_TEXTURE_2D, img->tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->w, img->h, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//input functions
	//takes keys so you can use either virtual key codes or the char value for letters
	bool getKeyDown(char key) {
		return in->GetKeyDown(key);
	}
	bool getKeyReleased(char key) {
		return in->getKeyReleased(key);
	}

};