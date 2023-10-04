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

/*struct int24 {
	uint8_t data[3];
	int24(int s) {
		data[0] = s & 255;
		data[1] = (s >> 8) & 255;
		data[2] = (s >> 16) & 255;
	}
	int24(char c, char c2, char c3) {
		data[0] = c;
		data[1] = c2;
		data[2] = c3;
	}
	operator float() const
	{
		return (float)(data[0] | (data[1] << 8) | (data[2] << 16));
	}
	operator char() const {
		return (char)(data[0] | (data[1] << 8) | (data[2] << 16));
	}
	operator short() const {
		return (short)(data[0] | (data[1] << 8) | (data[2] << 16));
	}
};*/


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

		bool writeData(BYTE* dat, size_t n) {
			if (n_write) {
				return false;
			}
			else if ((pos + (n * blockalign) >= size)) {
				size_t nt = (size - pos);
				//size_t nb = (nt * blockalign);
				std::memcpy(dat, data + pos, nt - 1);
				//std::cout << "write data over reads the file data, writing the remainder of file\n";
				n_write = true;
				return false;
			}
			std::memcpy(dat, data + pos, n * (blockalign));
			pos += (n * (blockalign));
			return true;
		}
	};

	class FileStream {
		bool n_write = false;
		size_t pos = 0;
		std::string file;
		int bytesp = 1;
		int blockalign = 0;
		std::ifstream fi;
	public:
		FileStream(std::string file) {
			fi.open(file, std::ios::binary);
			if (fi) {
				this->file = file;
				char c[40];
				fi.read(c, 32);
				pos += 32;
				short bl;
				fi.read((char*)&bl, 2);
				blockalign = (int)bl;
				pos += 2;
				fi.read((char*)&bl, 2);
				bytesp = (bl) / 8;
				pos += 2;
				//now find the data section
				strMatch("data");
				//skip the four bytes of data size
				fi.read(c, 4);
				pos += 4;
			}
		}
		~FileStream() {
			fi.close();
		}
		bool writeData(BYTE* dat, size_t n, WavBytes bits) {
			if (n_write) {
				return false;
			}
			void* d1 = std::malloc(n * blockalign);
			if (d1 == nullptr) {
				return false;
			}
			fi.read((char*)d1, n * blockalign);
			size_t tt;
			WavBytes w1 = (WavBytes)(bytesp);
			void* da1 = Translator::translate(d1, n * blockalign, &tt, w1, bits);
			if (da1 == nullptr) {
				std::memcpy(dat, d1, n * blockalign);
			}
			else {
				std::memcpy(dat, da1, tt);
				std::free(da1);
			}
			std::free(d1);
			if (!fi) {
				n_write = true;
			}
			return true;
		}
		bool strMatch(std::string str) {
			size_t i = 0;
			char c;
			while (true) {
				pos++;
				c = fi.get();
				if (c == str[i]) {
					i++;
					if (i >= str.size()) {
						return true;
					}
				}
				else {
					i = 0;
				}
				if (!fi) {
					break;
				}
			}
			return false;
		}
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
	~AudioStream() {
		CoTaskMemFree(format);
		SAFE_RELEASE(penum);
		SAFE_RELEASE(pdevice);
		SAFE_RELEASE(client);
		SAFE_RELEASE(render);
	}

	AudioStream();

	void playStream();
	void playFile(Audio file) {
		SoundP sp;
		sp.blockalign = file->blockalign;
		sp.data = file->data;
		sp.size = file->size;
		sound_files.push_back(sp);
	}
	void streamFile(std::string file) {
		FileStream* fs = new FileStream(file);
		stream_files.push_back(fs);		
	}
	void pause() {
		play = false;
		client->Stop();
	}
	void start() {
		play = true;
		client->Start();
	}
	void reset() {
		client->Reset();
	}
};


//https://habr.com/en/articles/663352/#windows-and-wasapi
//https://www.sounddevices.com/32-bit-float-files-explained/
//https://stackoverflow.com/questions/44759526/how-winapi-handle-iaudioclient-seteventhandle-works
//https://gist.github.com/Liastre/ff201f37bc62f6dc0b7f5541923565ab
//https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/audio/RenderExclusiveEventDriven/WASAPIRenderer.cpp
//problem is the audio i am trying to play isnt the same format as device format so we get no output, need to be able to convert any format to the another format
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
	AudioPlayer(size_t n_streams) {
		for (size_t i = 0; i < n_streams; i++) {
			AudioStream* as = new AudioStream;
			streams.push_back(as);
		}

		rend_thread = std::thread(&AudioPlayer::_RenderThread, this);
	}
	~AudioPlayer() {
		rend_thread.join();
		run = false;
		for (size_t i = 0; i < streams.size();i++) {
			AudioStream* as = streams[i];
			streams.erase(streams.begin() + i);
			delete as;
		}
		
	}
	Audio loadWavFile(std::string file);

	void playFile(std::string path, size_t stream);
	void playFile(Audio file, size_t stream);
	void pause(size_t stream);
	void start(size_t stream);
	void clear(size_t stream);
	void end();

	Audio generateSound();
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

	//image functions
	//mass draws an image based on buffer_2d
	void renderImgs(IMG img, bool blend);
	//rotates counter clockwise around top left point
	//mass draws an image with rotations
	void renderImgsRotated(IMG img, bool blend);
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
		img_vertexs.push_back({ x, y, 0.0f, 0.0f, 0.0f, 0, x, y });
		img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, 0, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, 0, x, y });

		//triangle 2
		img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, 0, x, y });
		img_vertexs.push_back({ x + w, y - h, 0.0f, 1.0f, 1.0f, 0, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, 0, x, y });

	}

	void addImageCall(float x, float y, float w, float h, float img_x, float img_y, float img_w, float img_h) {
		float imgx = img_x / img_w;
		float imgy = img_y / img_h;
		float imgw = img_w / img_w;
		float imgh = img_h / img_h;
		//triangle 1
		img_vertexs.push_back({ x, y, 0.0f, imgx, imgy, 0, x, y });
		img_vertexs.push_back({ x + w, y, 0.0f, imgx+imgw, imgy, 0, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy+imgh, 0, x, y });

		//triangle 2
		img_vertexs.push_back({ x + w, y, 0.0f, imgx+imgw, imgy, 0, x, y });
		img_vertexs.push_back({ x + w, y - h, 0.0f, imgx+imgw, imgy+imgh, 0, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy+imgh, 0, x, y });
	}

	float convertToRange(float n, float min, float max, float old_min, float old_max) {
		return ((n - old_min) / (old_max - old_min)) * (max - min) + min;
	}

	//angle in radians
	void addImageRotatedCall(float x, float y, float w, float h, float ang) {
		//triangle 1
		img_vertexs.push_back({ x, y, 0.0f, 0.0f, 0.0f, ang, x, y});
		img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, ang, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, ang, x, y });
		//triangle 2
		img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, ang, x, y });
		img_vertexs.push_back({ x + w, y - h, 0.0f, 1.0f, 1.0f, ang, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, ang, x, y });
	}

	void addImageRotatedCall(IMG img, float x, float y, float w, float h, float ang, float img_x, float img_y, float img_w, float img_h) {
		float imgx = img_x / img->w;
		float imgy = img_y / img->h;
		float imgw = img_w / img->w;
		float imgh = img_h / img->h;
		//triangle 1
		img_vertexs.push_back({ x, y, 0.0f, imgx, imgy, ang, x, y });
		img_vertexs.push_back({ x + w, y, 0.0f, imgx + imgw, imgy, ang, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy + imgh, ang, x, y });

		//triangle 2
		img_vertexs.push_back({ x + w, y, 0.0f, imgx + imgw, imgy, ang, x, y });
		img_vertexs.push_back({ x + w, y - h, 0.0f, imgx + imgw, imgy + imgh, ang, x, y });
		img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy + imgh, ang, x, y });
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
	Font loadFont(std::string file, UINT16 start, UINT16 end);
	void drawText(std::string text, Font* font, float x, float y, int ptsize);
	RasterGlyph rasterizeGlyph(Glyph* g, int w, int h, uint32_t color, bool flipx = false);
	void rasterizeFont(Font* font, int ptsize, uint32_t color, std::vector<UINT16> flipx);
	void drawRasterText(Font* font, std::string text, float x, float y, int ptsize);
	//audio functions
	//function loading
	//only run this after gl initilized
	void loadFunctions();
};