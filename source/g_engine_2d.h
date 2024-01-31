#pragma once
#include "font_renderer.h"

//add 3d support
//add 3d line rendering
//add 3d primitives
//voxel engine plug-in
//isometric engine plug-in


class delta{
	private:
	int64_t last;
	double freq;
	public:
	//call this every frame to update and get delta
	double getDelta(){
		LARGE_INTEGER ticks;
		if(!QueryPerformanceCounter(&ticks)){
			std::cout << "Failed to query performance counter!\n";
			return -1.0f;
		}
		return (double(ticks.QuadPart) - last)/freq;
	}
	delta(){
		LARGE_INTEGER li;
		if(!QueryPerformanceCounter(&li)){
			std::cout << "Failed to query performance counter!\n";
			return;
		}
		freq = double(li.QuadPart) / 1000.0;
		QueryPerformanceCounter(&li);
		last = double(li.QuadPart);
	}
};


//https://github.com/Ethan-Bierlein/SWOGLL/blob/master/SWOGLL.cpp
//https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

class EngineNewGL {
private:
	Window* wind;
	Input* in;
	std::function<void()> renderFund;
	//color constants
	vec4 draw_color;
	vec4 clear_color;

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

	float convertToRange(float n, float min, float max, float old_min, float old_max) {
		return ((n - old_min) / (old_max - old_min)) * (max - min) + min;
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
	double getDelta();
	//returns number of frames in a second and the average frame time in milliseconds, every second. 
	std::pair<double, double> getFrames();

	
	//function loading
	//only run this after gl initilized
	void loadFunctions();
};