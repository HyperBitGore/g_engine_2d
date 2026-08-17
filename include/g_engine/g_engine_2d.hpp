#pragma once
#include "rendering/font_renderer.hpp"
#include "rendering/image_renderer.hpp"
#include "rendering/primitive_renderer.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include "rendering/renderer.hpp"
#include "util/logging.hpp"
#include "util/matrix.hpp"
#include "util/shader.hpp"
#define MAINTAIN_ASPECT_RATIO_COMPONENT 0x1
#define USE_VIEW_MATRICE 0x2
#define ALL_COMPONENTS MAINTAIN_ASPECT_RATIO_COMPONENT | USE_VIEW_MATRICE

// unit testing
//	- opengl, test proper command streams, and test the color output is correct with drawpasses
//	- systems, test behavior of windows and audio is maintained

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
	gore::vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	//delta time
	std::chrono::duration<std::chrono::milliseconds::period> last_time;
	unsigned long long frames = 0;
	double delta = 0;
	double delta_f = 0;
	double frameRate = 30;
	double averageFrameTimeMilliseconds = 33.333;
	// rendering
	int texture_units;
	uint32_t component_mask = 0;
	std::unique_ptr<drawpass> dr1 = nullptr;
	std::unique_ptr<image_renderer> basic_image = nullptr;
	uint32_t target_width;
	uint32_t target_height;
	uint32_t window_width;
	uint32_t window_height;
	// frame rate related
	bool limitframes = false;
	float fps = 60.0f;
	float frame_time_limit = 0.16f;
	float accumlator = 0.0f;
	#if defined (__unix__)
	Display* display;
	GLXContext ctx;
	#endif
	#if defined (_WIN32)
	HGLRC ctx;
	#endif
	// matrices
	gore::matrix view = gore::matrix(4, 4);
	gore::matrix ortho = gore::matrix(4, 4);
	// renderers
	struct render_ptr {
		renderer_base* ptr;
		bool maintain_viewport;
		bool update_view;
		bool update_view_3d;
	};
	std::vector<render_ptr> renderers;
	std::function<void(uint32_t, uint32_t)> resize;
public:
	std::shared_ptr<gore::logger> logger;
	// parts is a bitmask which tells us what to load
	g_engine_2d(const char* window_name, uint32_t width, uint32_t height, uint32_t component_mask, gore::LogType log_level = gore::LogType::NONE, std::string log_file = "g_engine_2d.log", uint32_t target_width = 0, uint32_t target_height = 0);

	//move constructor
	g_engine_2d(g_engine_2d&& o) {
		this->logger = std::move(o.logger);
		this->wind = o.wind;
		this->in = o.in;
		this->renderFund = o.renderFund;
		this->clear_color = o.clear_color;
		this->dr1 = std::move(o.dr1);
		this->window_width = o.window_width;
		this->window_height = o.window_height;
		this->component_mask = o.component_mask;
		#if defined (__unix__)
		this->display = o.display;
		this->ctx = o.ctx;
		#endif
		this->view = o.view;
		this->ortho = o.ortho;
	}
	//copy constructor, probably not accurate to what behavior we would want out of a copy constructor
	g_engine_2d(const g_engine_2d& o) {
		this->logger = o.logger;
		this->wind = o.wind;
		this->in = o.in;
		this->renderFund = o.renderFund;
		this->clear_color = o.clear_color;
		this->dr1 = std::make_unique<drawpass>(*o.dr1);;
		this->window_width = o.window_width;
		this->window_height = o.window_height;
		this->component_mask = o.component_mask;
		#if defined (__unix__)
		this->display = o.display;
		this->ctx = o.ctx;
		#endif
		this->view = o.view;
		this->ortho = o.ortho;
	}
	~g_engine_2d () {
		#if defined (__unix__)
		glXMakeCurrent(display, None, NULL);
		glXDestroyContext(display, ctx);
		XCloseDisplay(display);
		#endif
	}
	void toggleFrameLimitActive ();
	void setFrameLimit (uint32_t fps);
	//sets renderfunction
	void setRenderFunction(std::function<void()> func) {
		renderFund = func;
	}
	// adds a renderer to be updated on window resize
	void addRenderer (renderer_base* ptr, bool maintain_viewport, bool update_view, bool update_view_3d) {
		renderers.push_back({ptr, maintain_viewport, update_view, update_view_3d});
		if (resize) {
			resize(window_width, window_height);
		}
	}
	// sets the window resize user function
	void setWindowResize(std::function<void(uint32_t, uint32_t)> func);
	//updates the window
	bool updateWindow();
	// set window title
	void setWindowTitle (std::string title);
	//toggles the window to fullscreen
	void toggleFullscreen();
	// toggle viewport resizing
	void setMaintainViewport(bool maintain);
	// bool center toggles whether to keep mouse in center of screen
	void toggleMouseCapture(bool center);
	// toggles whether to hide the mouse cursor
	void toggleMouseHide();
	// set mouse move function 
	void setMouseMoveFunction (std::function<void()> func);
	// get window dpi
	uint32_t getDPI();
	//input functions
	//takes keys so you can use either virtual key codes or the char value for letters
	bool getKeyDown(uint32_t key);
	bool getKeyReleased(uint32_t key);
	void updateInputState();

	vec2 getMousePos(bool raw = false, bool skip_view = false);
	//ease of use
	bool getMouseLeftDown() { return in->GetKeyDown(g_MouseLeft); }
	bool getMouseRightDown() { return in->GetKeyDown(g_MouseRight); }
	bool getMouseMiddleDown() { return in->GetKeyDown(g_MouseMiddle); }

	bool getMouseLeftReleased() { return in->GetKeyReleased(g_MouseLeft); }
	bool getMouseRightReleased() { return in->GetKeyReleased(g_MouseRight); }
	bool getMouseMiddleReleased() { return in->GetKeyReleased(g_MouseMiddle); }

	//color functions
	void setClearColor(vec4 c) {
		clear_color = c;
	}
	vec4 getClearColor() {
		return clear_color;
	}
	
	// camera helpers
	void updateView (float camera_x, float camera_y, float zoom);
	//delta time
	//returns the frame time in seconds
	double getDelta();
	//returns number of frames in a second and the average frame time in milliseconds, every second. 
	std::pair<double, double> getFrames();

	//enable/disable glthings
	void enable(GLenum en);
	void disable(GLenum en);
	// window access
	RAW_WINDOW getWindow () {
		return wind->getRawWindow();
	}
	RAW_DISPLAY getDisplay () {
		return wind->getRawDisplay();
	}
	// makes opengl context current for this engine
	void makeContextCurrent ();
};
}