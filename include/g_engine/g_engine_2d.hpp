#pragma once
#include "rendering/font_renderer.hpp"
#include "rendering/image_renderer.hpp"
#include "rendering/primitive_renderer.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include "util/logging.hpp"
#include "util/matrix.hpp"
#define PRIMITIVE_COMPONENT 0x1
#define IMAGE_COMPONENT 0x2
#define FONT_COMPONENT 0x4
#define GRAYSCALE_COMPONENT 0x8
#define MAINTAIN_ASPECT_RATIO_COMPONENT 0x10
#define USE_VIEW_MATRICE 0x20

#define ALL_COMPONENTS PRIMITIVE_COMPONENT | IMAGE_COMPONENT | FONT_COMPONENT | GRAYSCALE_COMPONENT | MAINTAIN_ASPECT_RATIO_COMPONENT | USE_VIEW_MATRICE

// add frame limiting
// add casting mouse pointer coords into viewspace
// add optimized image renderer

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
	uint32_t maintainRendererViewport = PRIMITIVE_COMPONENT | IMAGE_COMPONENT | FONT_COMPONENT | GRAYSCALE_COMPONENT;
	uint32_t component_mask = 0;
	std::unique_ptr<drawpass> dr1 = nullptr;
	std::unique_ptr<imagerenderer> basic_image = nullptr;
	uint32_t target_width;
	uint32_t target_height;
	uint32_t window_width;
	uint32_t window_height;
	// frame rate related
	bool limitframes = false;
	float fps = 60.0f;
	float frame_time_limit = 0.16f;
	float accumlator = 0.0f;
	//function loading
	//only run this after gl initilized
	void loadFunctions();
	#if defined (__unix__)
	Display* display;
	GLXContext ctx;
	#endif
	// matrices
	gore::matrix view = gore::matrix(4, 4);
	gore::matrix ortho = gore::matrix(4, 4);
public:
	// rendering backends
	std::unique_ptr<primitiverenderer> prim_r = nullptr;
	std::unique_ptr<imagerenderer> img_r = nullptr;
	std::unique_ptr<fontrenderer> font_renderer = nullptr;
	std::unique_ptr<grayscalerenderer> gray_r = nullptr;
	std::shared_ptr<gore::logger> logger;
	// parts is a bitmask which tells us what to load
	g_engine_2d(const char* window_name, uint32_t width, uint32_t height, uint32_t component_mask, gore::LogType log_level = gore::LogType::NONE, std::string log_file = "g_engine_2d.log", uint32_t target_width = 0, uint32_t target_height = 0);

	//move constructor
	g_engine_2d(g_engine_2d&& o) {
		this->prim_r = std::move(o.prim_r);
		this->img_r = std::move(o.img_r);
		this->font_renderer = std::move(o.font_renderer);
		this->gray_r = std::move(o.gray_r);
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
		this->prim_r = std::make_unique<primitiverenderer>(*o.prim_r);
		this->img_r = std::make_unique<imagerenderer>(*o.img_r);
		this->font_renderer = std::make_unique<fontrenderer>(*o.font_renderer);
		this->gray_r = std::make_unique<grayscalerenderer>(*o.gray_r);
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
	// toggle renderer viewport resizing, if the component is activated in mask will be resized on window resize
	void setRendererViewportMask (uint32_t mask);
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
	void setClearColor(vec4 c) {
		clear_color = c;
	}
	vec4 getClearColor() {
		return clear_color;
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
};
}