#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <gl/GL.h>
#include <glcorearb.h>
#include <wglext.h>
#include <wgl.h>
#include <functional>
#include <math.h>
#include "lodepng.h"
#define M_PI 3.14159265358979323846
#define pack_rgba(r,g,b,a) (uint32_t)(r<<24|g<<16|b<<8|a)
#define unpack_r(col) (uint8_t)((col>>24)&0xff)
#define unpack_g(col) (uint8_t)((col>>16)&0xff)
#define unpack_b(col) (uint8_t)((col >> 8)&0xff)
#define unpack_a(col) (uint8_t)(col&0xff)

// replace this with your favorite Assert() implementation
#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

static void FatalError(const char* message)
{
	MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
	ExitProcess(0);
}





//update to newer version of gl
//add shader support
//draw text
//add 3d support
//add 3d line rendering
//add 3d primitives
//voxel engine plug-in
//isometric engine plug-in






//added customizability and clean up
class Window {
private:
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	LPCWSTR class_name;
	int height;
	int width;
public:
	Window(LPCWSTR title, LPCWSTR CLASS_NAME, int h, int w, int x, int y);
	Window(const Window&) = delete;
	Window& operator =(const Window&) = delete;
	~Window() {
		//has to be same class name
		UnregisterClass(class_name, m_hinstance);
	}
	bool ProcessMessage();
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
	void setLastState();
	//input functions
	//probably switch to GetKeyboardState, so easier to use getKeyReleased
	void getState();


	bool GetKeyDown(char key);
	bool getKeyReleased(char key);
};


struct g_img {
	unsigned char* data;
	size_t tex;
	unsigned int w;
	unsigned int h;
};

typedef g_img* IMG;


class ImageLoader {
private:
	//this doesn't work
	static void readBMPPixels32(IMG f, std::stringstream& str, size_t offset, size_t raw_size);
	//fix color being off, ie read color table
	static void readBMPPixels24(IMG f, std::string str, size_t offset, size_t raw_size);
	static void parseBMPData(IMG f, std::stringstream& str, size_t offset, unsigned short bitsperpixel, size_t size);
public:
	//https://docs.fileformat.com/image/bmp/
	//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	//https://en.wikipedia.org/wiki/BMP_file_format
	//https://www.fileformat.info/format/bmp/egff.htm
	//https://medium.com/sysf/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
	static IMG loadBMP(std::string file);
	static IMG loadPNG(std::string file, unsigned int w, unsigned int h);
};


class EngineNewGL {
private:
	Window* wind;
	Input* in;
	HDC dc_w;
	HGLRC context;
	std::function<void()> renderFund;
public:
	EngineNewGL(LPCWSTR window_name, int width, int height);
	//port old functions into modern opengl
	

	//sets renderfunction
	void setRenderFunction(std::function<void()> func) {
		renderFund = func;
	}
};



class Engine {
private:
	Window* wind;
	Input* in;
	HDC dc_w;
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
		dc_w = GetDC(wind->getHwnd());
		//choosing pixel format
		int pixelFormat = ChoosePixelFormat(dc_w, &windowPixelFormatDesc);
		//set pixel format
		SetPixelFormat(dc_w, pixelFormat, &windowPixelFormatDesc);
		//now create opengl context
		context = wglCreateContext(dc_w);
		if (!wglMakeCurrent(dc_w, context)) {
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
		SwapBuffers(dc_w);
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
	void drawLineBetter(float x1, float y1, float x2, float y2) {
		glBegin(GL_POINTS);
		float dx = std::abs(x2 - x1);
		float dy = std::abs(y2 - y1);
		if (dx > dy) {
			if (x1 > x2) {
				float t = x1;
				x1 = x2;
				x2 = t;
				t = y1;
				y1 = y2;
				y2 = t;
			}
			float a = ((y2 - y1) / (x2 - x1)) / wind->getHeight();
			float d = y1;
			float inc = 1.0f / wind->getWidth();
			for (float x = x1; x <= x2; x+=inc) {
				glVertex2f(x, d);
				d = d + a;
			}

		}
		else {
			if (y1 > y2) {
				float t = x1;
				x1 = x2;
				x2 = t;
				t = y1;
				y1 = y2;
				y2 = t;
			}
			float a = ((x2 - x1) / (y2 - y1)) / wind->getWidth();
			float d = x1;
			float inc = 1.0f / wind->getHeight();
			for (float y = y1; y <= y2; y+=inc) {
				glVertex2f(d, y);
				d = d + a;
			}
		}
		glEnd();
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