#pragma once
#include "g_primitive_funcs.h"
#include "backend.h"
#include "matrix.h"
#include "vector.h"
#include "audio.h"
#include <thread>
#include <mutex>
#include <fstream>
#include <string>
#include <sstream>
#include "lodepng.h"




#define pack_rgba(r,g,b,a) (uint32_t)(r<<24|g<<16|b<<8|a)
#define pack_rgb(r,g,b) (uint32_t)(r<<24|g<<16|b<<8)
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





//optimize drawing
//	-remove uniform calls in draw calls, so move color setting to a different function
//  -reduce shader changes
//	-maybe switch to a seperate queue for all the calls and call of them at once
//add 3d support
//add 3d line rendering
//add 3d primitives
//voxel engine plug-in
//isometric engine plug-in

template<class T>
T clamp(T p, T n1, T n2) {
	if (p < n1) {
		p = n1;
	}
	else if (p > n2) {
		p = n2;
	}
	return p;
}



struct g_img {
	unsigned char* data;
	GLuint tex;
	unsigned int w;
	unsigned int h;
	int pos;
	uint8_t bytes_per_pixel;
};

typedef g_img* IMG;


class ImageLoader {
private:
	//this doesn't work
	static void readBMPPixels32(IMG f, std::stringstream& str, size_t offset, size_t raw_size);
	//fix color being off, ie read color table
	static void readBMPPixels24(IMG f, std::string str, size_t offset, size_t raw_size);
	static void parseBMPData(IMG f, std::stringstream& str, size_t offset, unsigned short bitsperpixel, size_t size);
	int cur_tex = 1;
	
public:
	//https://docs.fileformat.com/image/bmp/
	//https://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	//https://en.wikipedia.org/wiki/BMP_file_format
	//https://www.fileformat.info/format/bmp/egff.htm
	//https://medium.com/sysf/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
	
	IMG loadBMP(std::string file);
	IMG loadPNG(std::string file, unsigned int w, unsigned int h);
	//for when you create img data yourself and need to actually bind a texture easily
	void createTexture(IMG img, GLenum internalformat, GLenum format);
	//all of these assume the color componenets are 8 bits each
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g, uint8_t b);
	static void setPixel(IMG img, int x, int y, uint8_t r, uint8_t g);
	static void setPixel(IMG img, int x, int y, uint8_t r);
	//assumed the color components are 8 bits each
	static void setPixel(IMG img, int x, int y, uint32_t color, int bytes);



	static uint64_t getPixel(IMG img, int x, int y, int bytes);


	//generates an img struct with no texture assigned but with generated blank data
	static IMG generateBlankIMG(int w, int h, int bytes_per_pixel);
};




struct Point {
	int x;
	int y;
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
};

class Line {
private:
	
public:
	vec2 p1;
	vec2 p2;
	Line() {
		
	}
	Line(vec2 v1, vec2 v2) {
		p1 = v1;
		p2 = v2;
	}
	Line(const Line& l) {
		p1 = l.p1;
		p2 = l.p2;
	}
};

struct RasterGlyph {
	UINT16 c;
	IMG data; //img we rasterize to
};

struct Glyph {
	UINT16 c; //for unicode
	std::vector<Line> contours;
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

//store a bunch of images which you can stich together
//adding an image stiches it into the image
class ImageAtlas {
private:
	Gore::HashMap<Point, IMG> images;
	IMG img;
	int cur_y = 0;
	int cur_x = 0;
public:
	ImageAtlas(int w, int h, int bytes_per_pixel);
	void addImage(IMG img);
	Point getImagePos(IMG img);
	IMG getImg();
};


//throw hashmap in here for uniform lookup
class Shader {
private:
	GLuint program;
	Gore::HashMap<GLint, std::string> uniform_map;
	static int hash(std::string str) {
		size_t total = 0;
		for (size_t i = 0; i < str.size(); i++) {
			total += str[i];
		}
		return total % 30;
	}
public:
	Shader() {
		program = 0;
		uniform_map.setHashFunction(hash);
	}
	//copy constructor
	Shader(Shader& x) {
		this->program = x.program;
	}
	void bind();
	//need a bunch of these for every type, https://registry.khronos.org/OpenGL-Refpages/gl4/html/glUniform.xhtml
	//int overloads
	bool setuniform(std::string uni, GLint n);
	bool setuniform(std::string uni, GLint x, GLint y);
	bool setuniform(std::string uni, GLint x, GLint y, GLint z);
	bool setuniform(std::string uni, GLint x, GLint y, GLint z, GLint w);
	//unsigned int overloads
	bool setuniform(std::string uni, GLuint n);
	bool setuniform(std::string uni, GLuint x, GLuint y);
	bool setuniform(std::string uni, GLuint x, GLuint y, GLuint z);
	bool setuniform(std::string uni, GLuint x, GLuint y, GLuint z, GLuint w);
	//float overloads
	bool setuniform(std::string uni, GLfloat n);
	bool setuniform(std::string uni, vec2 n);
	bool setuniform(std::string uni, vec3 n);
	bool setuniform(std::string uni, vec4 n);
	//double overloads
	bool setuniform(std::string uni, GLdouble n);
	bool setuniform(std::string uni, GLdouble x, GLdouble y);
	bool setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z);
	bool setuniform(std::string uni, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	//array overloads
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLfloat* value);
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLint* value);
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLuint* value);
	bool setuniform(const std::string uni, const GLsizei stride, const GLsizei count, const GLdouble* value);
	//matrix overloads

	void compile(const char* vertex, const char* frag);
	void compile(const std::string vert_path, const std::string frag_path);
};


//https://open.gl/framebuffers
//https://www.youtube.com/watch?v=QQ3jr-9Rc1o
class DrawPass {
	private:
		GLuint color_buffer;
		GLuint depth_buffer; //also stencil buffer
		GLuint texture;
		GLenum attach;
	public:
		DrawPass(GLuint width, GLuint height, GLenum attach) {
			glGenFramebuffers_g(1, &color_buffer);
			glBindFramebuffer_g(GL_FRAMEBUFFER, color_buffer);
			glCreateTextures_g(GL_TEXTURE_2D, 1, &texture);
			//glBindTextureUnit_g(GL_TEXTURE_2D, texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D_g(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

			
			//creating render buffer
			glGenRenderbuffers_g(1, &depth_buffer);
			glBindRenderbuffer_g(GL_RENDERBUFFER, depth_buffer);
			glRenderbufferStorage_g(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer_g(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
			GLuint ret = glCheckFramebufferStatus_g(GL_FRAMEBUFFER);
			if (ret != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer failed creation!\n";
				std::cout << ret << "\n";
			}
			glBindFramebuffer_g(GL_FRAMEBUFFER, 0);

			this->attach = attach;
		}
		~DrawPass() {
			glDeleteFramebuffers_g(1, &color_buffer);
			glDeleteRenderbuffers_g(1, &depth_buffer);
			glDeleteTextures(1, &texture);
		}
		void bind() {
			glBindFramebuffer_g(GL_FRAMEBUFFER, color_buffer);
		}
		void unbind() {
			glBindFramebuffer_g(GL_FRAMEBUFFER, 0);
		}
		GLuint getTexture() {
			return texture;
		}
};



//switch to using multiple buffers so we can use all of the texture units on the gpu, but also have to dynamically generate the 
//https://www.khronos.org/opengl/wiki/Texture
	//-read the glsl binding section
class ImageRenderer {
private:
	struct ivertex{
		float x;
		float y;
		float uvx;
		float uvy;
	};
	std::vector<ivertex> vertexs;
	Shader shader;
	GLuint vao;
	GLuint vertex_buffer;
public:
	ImageRenderer(size_t w, size_t h);
	void addImageVertex(float x, float y, float w, float h);
	void drawBuffer();
	void drawImage(IMG img, float x, float y, float w, float h);
};

class PrimitiveRenderer {
private:
	std::vector<vec2> vertexs;
	GLuint vertex_buffer;
	GLuint allocated;
	Shader triangle_shader;
	GLuint triangle_vao;
	Shader point_shader;
	GLuint point_vao;
	Shader line_shader;
	GLuint line_vao;
public:
	//use to initialize shaders
	PrimitiveRenderer(GLuint sw, GLuint sh);
	//sets color for drawing
	void setColor(vec4 color);
	//triangles
	void addTriangle(vec2 v1, vec2 v2, vec2 v3);
	void drawTriangle(vec2 v1, vec2 v2, vec2 v3);
	void drawBufferTriangle();
	//quads
	void addQuad(vec2 pos, float w, float h);
	void drawQuad(vec2 pos, float w, float h);
	void drawBufferQuad();
	//points
	void addPoint(vec2 p);
	void drawPoint(vec2 p);
	void drawBufferPoint();
	//lines
	void addLine(vec2 p1, vec2 p2);
	void drawLine(vec2 p1, vec2 p2);
	void drawBufferLine();
	void setLineWidth(float l);
	//this add points to buffer, they are used for lines so make sure to call drawBufferLine when these are used
	void linearBezier(vec2 p1, vec2 p2);
	void quadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv);
	void cubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv);
	void circle(vec2 p, float r);
};

class FontRenderer {
	private:
	PrimitiveRenderer* pr;
	float convertToRange(float n, float min, float max, float old_min, float old_max) {
		return ((n - old_min) / (old_max - old_min)) * (max - min) + min;
	}
	public:
	FontRenderer(PrimitiveRenderer* pr){
		this->pr = pr;
	}
	//font functions
	Font loadFont(std::string file, UINT16 start, UINT16 end);
	void drawText(std::string text, Font* font, float x, float y, int ptsize);
	RasterGlyph rasterizeGlyph(Glyph* g, int w, int h, uint32_t color, bool flipx = false);
	void rasterizeFont(Font* font, int ptsize, uint32_t color, std::vector<UINT16> flipx);
	void drawRasterText(Font* font, std::string text, float x, float y, int ptsize);
};

class Renderer3D {

};


//https://github.com/Ethan-Bierlein/SWOGLL/blob/master/SWOGLL.cpp
//https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

class EngineNewGL {
private:
	Window* wind;
	Input* in;
	ImageLoader img_l;
	std::function<void()> renderFund;
	//color constants
	vec4 draw_color;
	vec4 clear_color;

	//Vertex buffers
	GLuint img_buffer;
	GLuint texture_buffer; //it's an ssbo! haha nice

	//vertex arrays
	GLuint VAO_Img;
	GLuint VAO_Imgr;

	//gl Shader programs
	GLuint shader_img;
	GLuint shader_imgr;

	//gl uniforms
	GLuint texuniform_imgr;
	GLuint texuniform_img;


	//vertex vectors
	std::vector<GLuint> imgs_drawn; //bind these textures because the user added them to be drawn
	std::vector<img_vertex> img_vertexs;

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
	int texture_units;

public:
	EngineNewGL(LPCSTR window_name, int width, int height);

	//move constructor
	EngineNewGL(EngineNewGL&& o) {
		
	}
	//copy constructor
	EngineNewGL(const EngineNewGL& o) {

	}

	//sets renderfunction
	void setRenderFunction(std::function<void()> func) {
		renderFund = func;
	}
	//updates the window
	bool updateWindow();

	//compiles shader from source
	GLuint compileShader(const char* vertex, const char* fragment);


	//framebuffer functions


	//image functions
	// 
	void bindImg(IMG img);

	void bindImg(GLuint img);
	//mass draws an image based on buffer_2d
	void renderImgs(bool blend);
	//rotates counter clockwise around top left point
	//mass draws an image with rotations
	void renderImgsRotated(bool blend);
	//run after you've done all the editing of data you want to
	void updateIMG(IMG img) {
		glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->w, img->h, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void createTexture(IMG img, GLenum internalformat, GLenum format) {
		img_l.createTexture(img, internalformat, format);
	}
	IMG loadPNG(std::string file, unsigned int w, unsigned int h) {
		return img_l.loadPNG(file, w, h);
	}
	IMG loadBMP(std::string file) {
		return img_l.loadBMP(file);
	}

	//input functions
	//takes keys so you can use either virtual key codes or the char value for letters
	bool getKeyDown(char key);
	bool getKeyReleased(char key);

	vec2 getMousePos();
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
	//image call functions
	void addImageCall(float x, float y, float w, float h);

	void addImageCall(float x, float y, float w, float h, float img_x, float img_y, float img_w, float img_h);

	float convertToRange(float n, float min, float max, float old_min, float old_max) {
		return ((n - old_min) / (old_max - old_min)) * (max - min) + min;
	}

	//angle in radians
	void addImageRotatedCall(float x, float y, float w, float h, float ang);

	void addImageRotatedCall(IMG img, float x, float y, float w, float h, float ang, float img_x, float img_y, float img_w, float img_h);

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
	double getDelta();
	//returns number of frames in a second and the average frame time in milliseconds, every second. 
	std::pair<double, double> getFrames();

	
	//function loading
	//only run this after gl initilized
	void loadFunctions();
};