#pragma once
#include "rendering/font_renderer.hpp"
#include "rendering/image_renderer.hpp"
#include "rendering/primitive_renderer.hpp"
#include <memory>
#include "util/logging.hpp"
#define PRIMITIVE_COMPONENT 0x1
#define IMAGE_COMPONENT 0x2
#define FONT_COMPONENT 0x4
#define GRAYSCALE_COMPONENT 0x8


//add 3d support
//add 3d line rendering
//add 3d primitives
//voxel engine plug-in
//isometric engine plug-in

// https://victorlecomte.com/cp-geo.pdf

//https://github.com/Ethan-Bierlein/SWOGLL/blob/master/SWOGLL.cpp
//https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
namespace gore {
class g_engine_2d {
private:
	g_window* wind;
	input* in;
	std::function<void()> renderFund;
	//color constants
	gore::vec4 draw_color;
	gore::vec4 clear_color;

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
	// rendering backends
	std::unique_ptr<primitiverenderer> prim_r = nullptr;
	std::unique_ptr<imagerenderer> img_r = nullptr;
	std::unique_ptr<fontrenderer> font_renderer = nullptr;
	std::unique_ptr<grayscalerenderer> gray_r = nullptr;
	std::shared_ptr<gore::logger> logger;
	// parts is a bitmask which tells us what to load
	g_engine_2d(const char* window_name, int width, int height, uint8_t component_mask, gore::LogType log_level = gore::LogType::NONE, std::string log_file = "g_engine_2d.log");

	//move constructor
	g_engine_2d(g_engine_2d&& o) {
		this->prim_r = std::move(o.prim_r);
		this->img_r = std::move(o.img_r);
		this->font_renderer = std::move(o.font_renderer);
		this->gray_r = std::move(o.gray_r);
		this->logger = std::move(o.logger);
		//this->ap = std::move(o.ap);
		this->wind = o.wind;
		this->in = o.in;
		this->renderFund = o.renderFund;
		this->draw_color = o.draw_color;
		this->clear_color = o.clear_color;
		#if defined (__unix__)
		this->display = o.display;
		this->ctx = o.ctx;
		#endif
	}
	//copy constructor, probably not accurate to what behavior we would want out of a copy constructor
	g_engine_2d(const g_engine_2d& o) {
		this->prim_r = std::make_unique<primitiverenderer>(*o.prim_r);
		this->img_r = std::make_unique<imagerenderer>(*o.img_r);
		this->font_renderer = std::make_unique<fontrenderer>(*o.font_renderer);
		this->gray_r = std::make_unique<grayscalerenderer>(*o.gray_r);
		this->logger = o.logger;
		//this->ap = o.ap;
		this->wind = o.wind;
		this->in = o.in;
		this->renderFund = o.renderFund;
		this->draw_color = o.draw_color;
		this->clear_color = o.clear_color;
		#if defined (__unix__)
		this->display = o.display;
		this->ctx = o.ctx;
		#endif
	}
	~g_engine_2d () {
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
	void setWindowResize(std::function<void(uint32_t, uint32_t)> func);
	//updates the window
	bool updateWindow();
	
	//toggles the window to fullscreen
	void toggleFullscreen();
	// toggle viewport resizing
	void toggleMaintainViewport();
	// bool center toggles whether to keep mouse in center of screen
	void toggleMouseCapture(bool center);
	// toggles whether to hide the mouse cursor
	void toggleMouseHide();
	// get window dpi
	uint32_t getDPI();
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
}