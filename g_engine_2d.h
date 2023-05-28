#pragma once

#include "gl_defines.h"
#include "g_primitive_funcs.h"
#include <fstream>
#include <string>
#include <sstream>
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



//draw text
//optimize drawing
//	-remove uniform calls in draw calls, so move color setting to a different function
//  -reduce shader changes
//	-maybe switch to a seperate queue for all the calls and call of them at once
//switch circle to entirly shader based instead of using lines
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
	bool GetKeyReleased(char key);

};


struct g_img {
	unsigned char* data;
	GLuint tex;
	unsigned int w;
	unsigned int h;
	int pos;
};

typedef g_img* IMG;


class ImageLoader {
private:
	//this doesn't work
	static void readBMPPixels32(IMG f, std::stringstream& str, size_t offset, size_t raw_size);
	//fix color being off, ie read color table
	static void readBMPPixels24(IMG f, std::string str, size_t offset, size_t raw_size);
	static void parseBMPData(IMG f, std::stringstream& str, size_t offset, unsigned short bitsperpixel, size_t size);
	int cur_tex = 0;
public:
	//https://docs.fileformat.com/image/bmp/
	//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	//https://en.wikipedia.org/wiki/BMP_file_format
	//https://www.fileformat.info/format/bmp/egff.htm
	//https://medium.com/sysf/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
	IMG loadBMP(std::string file);
	IMG loadPNG(std::string file, unsigned int w, unsigned int h);
	//for when you create img data yourself and need to actually bind a texture easily
	void createTexture(IMG img); 
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void setPixel(IMG img, int x, int y, uint32_t color);
	static uint32_t getPixel(IMG img, int x, int y);
	//generates an img struct with no texture assigned but with generated blank data
	static IMG generateBlankIMG(int w, int h);
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

struct img_vertex {
	float x;
	float y;
	float z;
	float uvx; //tex coord x
	float uvy; //tex coord y
	float rotation; //rotation in radians
	float rot_x; //point x to rotate around
	float rot_y; //point y to rotate around
	uint8_t r; //red mod value
	uint8_t g; //green mod value
	uint8_t b; //blue mod value
	uint8_t a; //alpha mod value
	//maybe add texture index, so we would have a big texture array instead of binding them seperataly everytime
};

struct Line {
	vec2 p1;
	vec2 p2;
};

struct RasterGlyph {
	UINT16 c;
	IMG data; //img we rasterize to
};

struct Glyph {
	UINT16 c; //for unicode
	std::vector<vec2> points;
	std::vector<Line> contours;
	std::vector<int> end_contours;
	short yMax;
	short yMin;
	short xMax;
	short xMin;
};

//speed this up with a hashmap when done
struct Font {
	std::string name;
	std::vector<Glyph> glyphs;
	std::vector<RasterGlyph> r_glyphs; //can ignore this if don't plan on using my rasterization
	int ptsize;
};


//https://github.com/Ethan-Bierlein/SWOGLL/blob/master/SWOGLL.cpp
//https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

class EngineNewGL {
private:
	Window* wind;
	Input* in;
	ImageLoader img_l;
	HDC dc_w;
	HGLRC context;
	std::function<void()> renderFund;
	//color constants
	vec4 draw_color;
	vec4 clear_color;


	//Vertex buffers
	GLuint vertex_buffer;
	GLuint uv_buffer;
	GLuint rot_buffer;
	GLuint rotpoint_buffer;

	//vertex arrays
	GLuint VAO_Triangle;
	GLuint VAO_Points;
	GLuint VAO_Line;
	GLuint VAO_Img;
	GLuint VAO_Imgr;

	//gl Shader programs
	GLuint shader_triangle2d;
	GLuint shader_point;
	GLuint shader_line;
	GLuint shader_img;
	GLuint shader_imgr;

	//gl uniforms
	GLuint texuniform_imgr;
	GLuint texuniform_img;

	GLuint coloruniform_tri;
	GLuint coloruniform_point;
	GLuint coloruniform_line;

	//uv vectors
	std::vector<vec2> buffer_uv;

	//vertex vectors
	std::vector<vec2> buffer_2d;
	std::vector<vec4> buffer_img;
	std::vector<img_vertex> img_buffer;

	//rotation vectors
	std::vector<vec2> rot_points;
	std::vector<float> rotations;

	//delta time
	clock_t delta = 0;
	clock_t delta_f = 0;
	clock_t begin_f;
	clock_t end_f;
	size_t frames = 0;
	double frameRate = 30;
	double averageFrameTimeMilliseconds = 33.333;
	double clockToMilliseconds(clock_t ticks) {
		// units/(units/time) => time (seconds) * 1000 = milliseconds
		return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
	}


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

	//adds quadratic bezier points but doesnt do a draw call
	void addquadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv);
	//draws a quadratic bezier curve
	void quadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv);
	//draws a cubic bezier curve
	void cubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv);
	//draws a circle
	void drawCircle(float x, float y, float r);
	//draws a quad
	void drawQuad(float x, float y, float w, float h);
	//draws quads from buffer_2d
	void drawQuads();

	//draw line of points
	void drawLinePoints(vec2 p1, vec2 p2);

	void addLinePoints(vec2 p1, vec2 p2);

	//draws a line
	void drawLine(float x1, float y1, float x2, float y2, float width);

	//draws lines from buffer_2d
	void drawLines(float width);

	//image functions
	//draws an img once
	void renderImg(IMG img, float x, float y, float w, float h);
	//mass draws an image based on buffer_2d
	void renderImgs(IMG img);
	//rotates counter clockwise around top left point
	void renderImgRotated(IMG img, float x, float y, float w, float h, float ang);
	//mass draws an image with rotations
	void renderImgsRotated(IMG img);
	//run after you've done all the editing of data you want to
	void updateIMG(IMG img) {
		glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->w, img->h, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void createTexture(IMG img) {
		img_l.createTexture(img);
	}
	IMG loadPNG(std::string file, unsigned int w, unsigned int h) {
		return img_l.loadPNG(file, w, h);
	}
	IMG loadBMP(std::string file) {
		return img_l.loadBMP(file);
	}

	//input functions
	//takes keys so you can use either virtual key codes or the char value for letters
	bool getKeyDown(char key) {
		return in->GetKeyDown(key);
	}
	bool getKeyReleased(char key) {
		return in->GetKeyReleased(key);
	}

	vec2 getMousePos() {
		vec2 p;
		LPPOINT po = new tagPOINT;
		GetCursorPos(po);
		ScreenToClient(wind->getHwnd(), po);
		p.x = (float)po->x;
		p.y = (float)po->y;
		//gotta translate the y axis for my coord system
		p.y = p.y - wind->getHeight();
		p.y = std::abs(p.y);

		delete po;
		return p;
	}
	//ease of use
	bool getMouseLeftDown() { return in->GetKeyDown(VK_LBUTTON); }
	bool getMouseRightDown() { return in->GetKeyDown(VK_RBUTTON); }
	bool getMouseMiddleDown() { return in->GetKeyDown(VK_MBUTTON); }

	bool getMouseLeftReleased() { return in->GetKeyReleased(VK_LBUTTON); }
	bool getMouseRightReleased() { return in->GetKeyReleased(VK_RBUTTON); }
	bool getMouseMiddleReleased() { return in->GetKeyReleased(VK_MBUTTON); }

	//color functions
	void setDrawColor(vec4 c) {
		draw_color = c;
	}
	void setClearColor(vec4 c) {
		clear_color = c;
	}
	vec4 getDrawColor() {
		return draw_color;
	}
	vec4 getClearColor() {
		return clear_color;
	}

	//vertice functions
	void addQuad(float x, float y, float w, float h) {
		//triangle 1
		buffer_2d.push_back({ x, y });
		buffer_2d.push_back({ x + w, y });
		buffer_2d.push_back({ x, y + h });
		//triangle 2
		buffer_2d.push_back({ x + w, y });
		buffer_2d.push_back({ x + w, y + h });
		buffer_2d.push_back({ x, y + h });
	}
	void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
		buffer_2d.push_back({ x1, y1 });
		buffer_2d.push_back({ x2, y2 });
		buffer_2d.push_back({ x3, y3 });
	}

	void add2DPoint(float x, float y) {
		buffer_2d.push_back({ x, y });
	}
	void add2DPoints(std::vector<vec2> points) {
		buffer_2d.insert(buffer_2d.end(), points.begin(), points.end());
	}
	//image call functions
	void addImageCall(float x, float y, float w, float h) {
		//triangle 1
		buffer_2d.push_back({ x, y });
		buffer_2d.push_back({ x + w, y });
		buffer_2d.push_back({ x, y - h });
		//triangle 2
		buffer_2d.push_back({ x + w, y });
		buffer_2d.push_back({ x + w, y - h });
		buffer_2d.push_back({ x, y - h });
		//uv triangle 1
		buffer_uv.push_back({ 0, 0 });
		buffer_uv.push_back({ 1, 0 });
		buffer_uv.push_back({ 0, 1 });
		//uv triangle 2
		buffer_uv.push_back({ 1, 0 });
		buffer_uv.push_back({ 1, 1 });
		buffer_uv.push_back({ 0, 1 });
	}
	//angle in radians
	void addImageRotatedCall(float x, float y, float w, float h, float ang) {

		
		int b = buffer_2d.size();
		//triangle 1
		buffer_2d.push_back({ x, y });
		buffer_2d.push_back({ x + w, y });
		buffer_2d.push_back({ x, y - h });
		//triangle 2
		buffer_2d.push_back({ x + w, y });
		buffer_2d.push_back({ x + w, y - h });
		buffer_2d.push_back({ x, y - h });

		for (size_t i = (size_t)b; i < buffer_2d.size(); i++) {
			float out_x = buffer_2d[i].x / float(wind->getWidth());
			float out_y = buffer_2d[i].y / float(wind->getHeight());
			out_x = (out_x * 2.0f) - 1;
			out_y = (out_y * 2.0f) - 1;

			float rotx = buffer_2d[i].x / float(wind->getWidth());
			float roty = buffer_2d[i].y / float(wind->getHeight());
			rotx = (rotx * 2.0f) - 1;
			roty = (roty * 2.0f) - 1;


			out_x = (out_x - rotx);
			out_y = (out_y - roty);

			out_x = out_x * cos(ang) - out_y * sin(ang);
			out_y = out_y * cos(ang) + out_x * sin(ang);

			out_x += rotx;
			out_y += roty;
			buffer_2d[i].x = out_x;
			buffer_2d[i].y = out_y;
		}


		//uv triangle 1
		buffer_uv.push_back({ 0, 0 });
		buffer_uv.push_back({ 1, 0 });
		buffer_uv.push_back({ 0, 1 });
		//uv triangle 2
		buffer_uv.push_back({ 1, 0 });
		buffer_uv.push_back({ 1, 1 });
		buffer_uv.push_back({ 0, 1 });
		//rotations
		rotations.push_back(ang);
		rotations.push_back(ang);
		rotations.push_back(ang);
		rotations.push_back(ang);
		rotations.push_back(ang);
		rotations.push_back(ang);
		//rot points
		rot_points.push_back({ buffer_2d[b].x, buffer_2d[b].y});
		rot_points.push_back({ buffer_2d[b].x, buffer_2d[b].y });
		rot_points.push_back({ buffer_2d[b].x, buffer_2d[b].y });
		rot_points.push_back({ buffer_2d[b].x, buffer_2d[b].y });
		rot_points.push_back({ buffer_2d[b].x, buffer_2d[b].y });
		rot_points.push_back({ buffer_2d[b].x, buffer_2d[b].y });
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
	
	//delta time
	//returns the frame time in seconds
	double getDelta() {
		clock_t d = delta;
		delta = 0;
		return d / (double)CLOCKS_PER_SEC;
	}
	//returns number of frames in a second and the average frame time in milliseconds, every second. 
	std::pair<double, double> getFrames() {
		if (clockToMilliseconds(delta_f) > 1000.0) { //every second
			frameRate = (double)frames * 0.5 + frameRate * 0.5; //more stable
			//std::cout << "Frames: " << frameRate << "\n";
			frames = 0;
			delta_f -= CLOCKS_PER_SEC;
			averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);
			//std::cout << "CPU time was:" << averageFrameTimeMilliseconds << std::endl;
			return { frameRate, averageFrameTimeMilliseconds };
		}
		return { frameRate, averageFrameTimeMilliseconds};
	}
	//font functions
	Font loadFont(std::string file);
	void drawText(std::string text, Font font, int ptsize);
	RasterGlyph rasterizeGlyph(Glyph* g, int w, int h, uint32_t color);
	void rasterizeFont(Font* font, int ptsize, uint32_t color);
	void drawRasterText(Font* font, std::string text, float x, float y, int ptsize);
	//function loading
	//only run this after gl initilized
	void loadFunctions();
};