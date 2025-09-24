#pragma once
#include "rendering/font_renderer.hpp"

//add 3d support
//add 3d line rendering
//add 3d primitives
//voxel engine plug-in
//isometric engine plug-in


//https://github.com/Ethan-Bierlein/SWOGLL/blob/master/SWOGLL.cpp
//https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions

class EngineNewGL {
private:
	g_window* wind;
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
	//function loading
	//only run this after gl initilized
	void loadFunctions();
	#if defined (__unix__)
	Display* display;
	GLXContext ctx;
	#endif
public:
	// parts is a bitmask which tells us what to load
	EngineNewGL(const char* window_name, int width, int height);

	//move constructor
	EngineNewGL(EngineNewGL&& o) {
		
	}
	//copy constructor
	EngineNewGL(const EngineNewGL& o) {

	}
	~EngineNewGL () {
		#if defined (__unix__)
		glXMakeCurrent(display, None, NULL);
		glXDestroyContext(display, ctx);
		XCloseDisplay(display);
		#endif
	}

	//sets renderfunction
	void setRenderFunction(std::function<void()> func) {
		renderFund = func;
	}
	// sets the window resize user function
	void setWindowResize(std::function<void(uint32_t, uint32_t)> func) {
		wind->setWindowResize(func);
	}
	//updates the window
	bool updateWindow();
	
	void toggleFullscreen();
	//input functions
	//takes keys so you can use either virtual key codes or the char value for letters
	bool getKeyDown(uint32_t key);
	bool getKeyReleased(uint32_t key);
	void updateInputState();

	vec2 getMousePos();
	//ease of use
	bool getMouseLeftDown() { return in->GetKeyDown(g_MouseLeft); }
	bool getMouseRightDown() { return in->GetKeyDown(g_MouseRight); }
	bool getMouseMiddleDown() { return in->GetKeyDown(g_MouseMiddle); }

	bool getMouseLeftReleased() { return in->GetKeyReleased(g_MouseLeft); }
	bool getMouseRightReleased() { return in->GetKeyReleased(g_MouseRight); }
	bool getMouseMiddleReleased() { return in->GetKeyReleased(g_MouseMiddle); }

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

	//enable/disable glthings
	void enable(GLenum en);
	void disable(GLenum en);
};