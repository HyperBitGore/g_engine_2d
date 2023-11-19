#pragma once

#include "gl_defines.h"
#include "g_primitive_funcs.h"
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "lodepng.h"


#define pack_rgba(r,g,b,a) (uint32_t)(r<<24|g<<16|b<<8|a)
#define pack_rgb(r,g,b) (uint32_t)(r<<24|g<<16|b<<8)
#define unpack_r(col) (uint8_t)((col>>24)&0xff)
#define unpack_g(col) (uint8_t)((col>>16)&0xff)
#define unpack_b(col) (uint8_t)((col >> 8)&0xff)
#define unpack_a(col) (uint8_t)(col&0xff)
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

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
	~Window();
	bool ProcessMessage();
	int getWidth();
	int getHeight();
	HWND getHwnd();
	bool updateWindow();
	bool swapBuffers();

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


//PCM data
struct Sound {
public:
	std::string name; //file name
	BYTE samplebits; //number of bits per sample
	BYTE channels; //number of channels
	int framesize; //frame size
	int blockalign; //block align for samples
	size_t size; //in bytes
	char* data; //actual wave form data
};
typedef Sound* Audio;

enum class WavBytes {
	BYTE8 = 1, BYTE16 = 2, BYTE24 = 3, BYTE32 = 4 
};

class AudioStream {
private:
	struct SoundP {
	private:
		size_t pos = 0;
		bool n_write = false;
	public:
		int blockalign;
		size_t size; //in bytes
		char* data; //actual wave form data
		int bytesp = 1; //bytes per sample

		bool writeData(BYTE* dat, size_t n, WavBytes bits);
	};

	class FileStream {
		bool n_write = false;
		size_t pos = 0;
		std::string file;
		int bytesp = 1;
		int blockalign = 0;
		std::ifstream fi;
	public:
		FileStream(std::string file);
		~FileStream();
		bool writeData(BYTE* dat, size_t n, WavBytes bits);
		bool strMatch(std::string str);
	};
	//https://stackoverflow.com/questions/74596138/microsoft-wasapi-do-different-audio-formats-need-different-data-in-the-buffer
	//translates to whatever format u need from input data
	class Translator {
	private:
		static int convertRange(int n, int min, int max, int n_min, int n_max);
		static float convertRange(float n, float min, float max, float n_min, float n_max);
		static short convertRange(short n, short min, short max, short n_min, short n_max);
		//redone
		static void convertToFloat(uint8_t* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertToFloat(short* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convert24ToFloat(uint8_t* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo16bit(char* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo16bit(float* mem, size_t size, void* n_mem, size_t n_size);
		//redo 24 bit
		static void convert24To16bit(char* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo8bit(short* mem, size_t size, void* n_mem, size_t n_size);
		//redo 24 bit
		static void convert24To8bit(char* mem, size_t size, void* n_mem, size_t n_size);
		//redone
		static void convertTo8bit(float* mem, size_t size, void* n_mem, size_t n_size);
		//redo this
		static void convertTo24bit(uint8_t* mem, size_t size, void* n_mem, size_t n_size);
		static void convertTo24bit(short* mem, size_t size, void* n_mem, size_t n_size);
		static void convertTo24bit(float* mem, size_t size, void* n_mem, size_t n_size);

	public:
		//make sure to free the memory returned here
		static void* translate(void* mem, size_t size, size_t* n_size, WavBytes org_bytes, WavBytes new_bytes);
	};


	WAVEFORMATEX* format = nullptr;
	IAudioClient* client = nullptr;
	IAudioRenderClient* render = nullptr;
	ISimpleAudioVolume* volume = nullptr;
	IMMDevice* pdevice = nullptr;
	IMMDeviceEnumerator* penum = nullptr;
	UINT32 buffer_size = 0;
	std::vector<SoundP> sound_files;
	std::vector<FileStream*> stream_files;

	HANDLE bufReady;
	HANDLE shutdown; //add later
	HANDLE paused;
	bool play = true;
	bool fs = false;
public:
	~AudioStream();
	AudioStream();

	void playStream();
	void playFile(Audio file);
	void streamFile(std::string file);
	void pause();
	void start();
	void reset();
};


//https://habr.com/en/articles/663352/#windows-and-wasapi
//https://www.sounddevices.com/32-bit-float-files-explained/
//https://stackoverflow.com/questions/44759526/how-winapi-handle-iaudioclient-seteventhandle-works
//https://gist.github.com/Liastre/ff201f37bc62f6dc0b7f5541923565ab
//https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/audio/RenderExclusiveEventDriven/WASAPIRenderer.cpp
class AudioPlayer {
private:
	struct PAudio {
		Audio aud;
		size_t stream;
	};
	struct FStream {
		std::string file;
		size_t stream;
	};

	struct AudioCommand {
		size_t type;
		size_t stream;
	};

	

	std::vector<PAudio> sound_files;
	std::vector<FStream> stream_files;
	std::vector<AudioStream*> streams;
	std::vector<AudioCommand> commands;

	std::thread rend_thread;
	std::mutex mtx;
	bool run = true;
	void _RenderThread();
public:
	AudioPlayer(size_t n_streams);
	~AudioPlayer();
	Audio loadWavFile(std::string file);

	void playFile(std::string path, size_t stream);
	void playFile(Audio file, size_t stream);
	void pause(size_t stream);
	void start(size_t stream);
	void clear(size_t stream);
	void end();

	//done
	Audio generateSin(size_t length, float freq, size_t sample_rate);
	//done
	Audio generateSquare(size_t length, float freq, size_t sample_rate);
	Audio generateTriangle(size_t length, float freq, size_t sample_rate);
	Audio generateSawtooth(size_t length, float freq, size_t sample_rate);
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
	//need a bunch of these for every type
	bool setuniform(std::string uni, GLint n);
	void compile(const char* vertex, const char* frag);
	void compile(const std::string vert_path, const std::string frag_path);
};

//switch to using multiple buffers so we can use all of the texture units on the gpu, but also have to dynamically generate the 
class ImageRenderer {
private:
	std::vector<img_vertex> buffer;
	Shader shader;
public:
	ImageRenderer() {
		buffer.reserve(1000);
	}
	void addImageVertex(float x, float y, float w, float h);
	void drawBuffer();
	void drawImage(IMG img, float x, float y, float w, float h);
};

class PrimitiveRenderer {

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
	GLuint vertex_buffer;
	GLuint img_buffer;
	GLuint texture_buffer; //it's an ssbo! haha nice

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

	//vertex vectors
	std::vector<vec2> buffer_2d;
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
	EngineNewGL(LPCWSTR window_name, int width, int height);

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
	//adds line points to buffer but doesn't draw them
	void addLinePoints(vec2 p1, vec2 p2);

	//draws a line
	void drawLine(float x1, float y1, float x2, float y2, float width);

	//draws lines from buffer_2d
	void drawLines(float width);

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

	//vertice functions
	void addQuad(float x, float y, float w, float h);
	void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

	void add2DPoint(float x, float y);
	void add2DPoints(std::vector<vec2> points);
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

	//font functions
	Font loadFont(std::string file, UINT16 start, UINT16 end);
	void drawText(std::string text, Font* font, float x, float y, int ptsize);
	RasterGlyph rasterizeGlyph(Glyph* g, int w, int h, uint32_t color, bool flipx = false);
	void rasterizeFont(Font* font, int ptsize, uint32_t color, std::vector<UINT16> flipx);
	void drawRasterText(Font* font, std::string text, float x, float y, int ptsize);
	//function loading
	//only run this after gl initilized
	void loadFunctions();
};