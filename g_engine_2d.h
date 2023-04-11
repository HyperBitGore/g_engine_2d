#pragma once

#include "gl_defines.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>
#define _USE_MATH_DEFINES
#include <math.h>
#include "lodepng.h"
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
	//input functions
	void setLastState();
	//probably switch to GetKeyboardState, so easier to use getKeyReleased
	void getState();


	bool GetKeyDown(char key);
	bool getKeyReleased(char key);
};


struct g_img {
	unsigned char* data;
	GLuint tex;
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
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void setPixel(IMG img, int x, int y, uint32_t color);
	static uint32_t getPixel(IMG img, int x, int y);
};

struct vec2 {
	float x;
	float y;
};

struct vec3 {
	float x;
	float y;
	float z;
};

struct vec4 {
	float x;
	float y;
	float z;
	float w;
};

//https://github.com/Ethan-Bierlein/SWOGLL/blob/master/SWOGLL.cpp
//https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

class EngineNewGL {
private:
	Window* wind;
	Input* in;
	HDC dc_w;
	HGLRC context;
	std::function<void()> renderFund;
	//color constants


	//Vertex buffers
	GLuint vertex_buffer;
	GLuint uv_buffer;

	//vertex arrays
	GLuint VAO_Triangle;
	GLuint VAO_Points;
	GLuint VAO_Line;
	GLuint VAO_Img;

	//gl Shader programs
	GLuint shader_2d;
	GLuint shader_point;
	GLuint shader_line;
	GLuint shader_img;

	//uv vectors
	std::vector<vec2> buffer_uv;

	//vertex vectors
	std::vector<vec2> buffer_2d;
	std::vector<vec4> buffer_img;
	std::vector<vec4> buffer_3d;

public:
	EngineNewGL(LPCWSTR window_name, int width, int height);
	//sets renderfunction
	void setRenderFunction(std::function<void()> func) {
		renderFund = func;
	}
	//updates the window
	bool updateWindow();

	//compiles shader from source
	GLuint compileShader(const char* vertex, const char* fragment);

	//2d drawing functions
	//draws a basic triangle
	void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
	//draws triangles from buffer_2d
	void drawTriangles();

	//draws a point
	void drawPoint(float x1, float y1);
	//draws points from the buffer_2d
	void drawPoints();

	//draws a circle
	void drawCircle(float x, float y, float r);
	//draws a quad
	void drawQuad(float x, float y, float w, float h);
	//draws quads from buffer_2d
	void drawQuads();

	//draws a line
	void drawLine(float x1, float y1, float x2, float y2, float width);

	//draws lines from buffer_2d
	void drawLines(float width);

	//image functions
	//draws an img once
	void renderImg(IMG img, float x, float y, float w, float h);
	//mass draws an image based on buffer_2d
	void renderImgs(IMG img, int w, int h);

	//rotates counter clockwise around top left point
	void renderImgRotated(IMG img, float x, float y, int w, int h, int ang);
	//run after you've done all the editing of data you want to
	void updateIMG(IMG img) {
		glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
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

	//vertice functions
	void add2DPoint(float x, float y) {
		buffer_2d.push_back({ x, y });
	}
	void add2DPoints(std::vector<vec2> points) {
		buffer_2d.insert(buffer_2d.end(), points.begin(), points.end());
	}


	//3d drawing functions
	//
	void drawPoint3D();
	//
	void drawTriangle3D();
	//
	void drawCube();
	//
	void drawSphere();
	//
	void drawLine3D();
	
	//function loading
	//only run this after gl initilized
	void loadFunctions();
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
	bool updateWindow();
	//primitive rendering
	void drawTriangle(float x, float y, float size);
	void drawCircle(float x, float y, float r);
	void drawPoint(float x, float y);
	void drawRectangle(float x, float y, float w, float h);
	void drawLine(float x1, float y1, float x2, float y2);
	void drawLineBetter(float x1, float y1, float x2, float y2);

	//image functions
	void renderImg(IMG img, float x, float y, int w, int h);
	//rotates counter clockwise around top left point
	void renderImgRotated(IMG img, float x, float y, int w, int h, int ang);
	//run after you've done all the editing of data you want to
	void updateIMG(IMG img) {
		glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
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