#include "g_engine_2d.hpp"
#include "gl_defines.hpp"
#include "util/matrix.hpp"
#include <GL/gl.h>
#include <chrono>
#include <cstdint>
#include <thread>

bool gore::g_engine_2d::getKeyDown(uint32_t key) {
	return in->GetKeyDown(key);
}
bool gore::g_engine_2d::getKeyReleased(uint32_t key) {
	return in->GetKeyReleased(key);
}

void gore::g_engine_2d::updateInputState(){
	in->updateState();
}

gore::vec2 gore::g_engine_2d::getMousePos(bool raw) {
	vec2 p;
	#if defined(_WIN32)
	LPPOINT po = new tagPOINT;
	GetCursorPos(po);
	ScreenToClient(wind->getRawWindow(), po);
	p.x = (float)po->x;
	p.y = (float)po->y;
	//gotta translate the y axis for my coord system
	p.y = p.y - wind->getHeight();
	p.y = std::abs(p.y);
	#endif
	#if defined(__unix__)
	int x, y, win_x, win_y;
	Window root_return, child_return;
	unsigned int mask_return;
	bool queryReturn = XQueryPointer(display, wind->getRawWindow(),
		&root_return, &child_return,
		&x, &y,       // Root (global) coords
		&win_x, &win_y, // Window-relative coords
		&mask_return);
	p.x = (float)win_x;
	p.y = (float)win_y;
	#endif

	#if defined(_WIN32)
	delete po;
	#endif
	if (raw) {
		return p;
	}
	if (component_mask & MAINTAIN_ASPECT_RATIO_COMPONENT) {
		float ppx = (p.x) / window_width;
		ppx = ppx * target_width;
		p.x = ppx;
		float ppy = (float)p.y / window_height;
		ppy = ppy * target_height;
		p.y = ppy;
	}
	if (component_mask & USE_VIEW_MATRICE) {
		gore::matrix inverseView = view.inverse();
    	gore::vec4 point = inverseView * gore::vec4(p.x, p.y, 1.0, 1.0);
		p.x = point.x;
		p.y = point.y;
	}
	return p;
}

//returns the frame time in seconds
double gore::g_engine_2d::getDelta() {
	return delta / 1000.0;
}
//returns number of frames in a second and the average frame time in milliseconds, every second. 
std::pair<double, double> gore::g_engine_2d::getFrames() {
	if (delta_f > 1000.0) { //every second
		frameRate = (double)frames * 0.5 + frameRate * 0.5; //more stable
		frames = 0;
		delta_f -= 1000.0;
		averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);
		return { frameRate, averageFrameTimeMilliseconds };
	}
	return { frameRate, averageFrameTimeMilliseconds };
}
// https://medium.com/@tglaiel/how-to-make-your-game-run-at-60fps-24c61210fe75

//utility type functions
bool gore::g_engine_2d::updateWindow() {
	auto begin_time = std::chrono::steady_clock::now();
	wind->updateWindow();
	if (!wind->ProcessMessage()) {
		logger->log("Closing window");
		delete wind;
		return false;
	}
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// this will probably compile more efficiently than two ifs
	if (component_mask & MAINTAIN_ASPECT_RATIO_COMPONENT) {
		glViewport(0, 0, this->target_width, this->target_height);
		dr1->clear();
		dr1->bind();
		renderFund();
		dr1->unbind();
		glViewport(0, 0, this->window_width, this->window_height);
		// blit to screen
		//basic_image->drawTexture(dr1->getTexture(), {0.0f, 0.0f}, {(float)this->window_width, (float)this->window_height}, {0.0f, 1.0f, 1.0f, -1.0f});
		// basic_image->drawTexture(dr1->getTexture(), {-1.0f, 1.0f}, {2.0f, -2.0f}, {0.0f, 1.0f, 1.0f, -1.0f});
		glBindFramebuffer(GL_READ_FRAMEBUFFER, dr1->getColorBuffer());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // screen framebuffer
		glBlitFramebuffer(0, 0, target_width, target_height, 0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	} else {
		renderFund();
	}
	

	if (!wind->swapBuffers())
	{
		FatalError("Failed to swap OpenGL buffers!");
		return false;
	}
	auto end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> dt = end_time - begin_time;
	delta = dt.count();
	delta_f += dt.count();
	frames++;
	// this is apparently how UE5 and unity do it
	if (limitframes) {
		double remainder = frame_time_limit - delta;
		if (remainder > 0.0) {
			constexpr double spin_threshold_ms = 4.0;
			// we sleep for a bit, but this is inaccurate so only sleep for portion and active wait for remainder
			if (remainder > spin_threshold_ms) {
				std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(remainder - spin_threshold_ms));
			}
			auto spin_target = end_time + std::chrono::duration<double, std::milli>(remainder);
			while (std::chrono::steady_clock::now() < spin_target) {}
			std::chrono::duration<double, std::milli> actual_dt = std::chrono::steady_clock::now() - begin_time;
			delta = actual_dt.count();
			delta_f += actual_dt.count() - dt.count();
		}
	}
	return true;
}


void gore::g_engine_2d::enable(GLenum en){
	glEnable(en);
}
void gore::g_engine_2d::disable(GLenum en){
	glDisable(en);
}

void gore::g_engine_2d::toggleFullscreen() {
	wind->toggleFullscreen();
}

void gore::g_engine_2d::toggleMouseCapture(bool center) {
	wind->captureMouseToggle(center);
}
void gore::g_engine_2d::setMouseMoveFunction (std::function<void()> func) {
	wind->setMouseMoveFunction(func);
}

void gore::g_engine_2d::toggleMouseHide() {
	wind->mouseHideToggle();	
}

uint32_t gore::g_engine_2d::getDPI() {
	return wind->getDPI();
}

void gore::g_engine_2d::setWindowResize(std::function<void(uint32_t, uint32_t)> func) {
	std::function<void(uint32_t, uint32_t)> f = [this, func](uint32_t w, uint32_t h) {
		if (this->component_mask & MAINTAIN_ASPECT_RATIO_COMPONENT) {
			basic_image->setDimensions(w, h);
		} else {
			if (triangle_r && line_r && point_r && (maintainRendererViewport & PRIMITIVE_COMPONENT)) {
				triangle_r->setDimensions(w, h);
				line_r->setDimensions(w, h);
				point_r->setDimensions(w, h);
			}
			if (img_r && (maintainRendererViewport & IMAGE_COMPONENT)) {
				img_r->setDimensions(w, h);
			}
			if (gray_r && (maintainRendererViewport & GRAYSCALE_COMPONENT)) {
				gray_r->setDimensions(w, h);
			}
			if (font_r && (maintainRendererViewport & FONT_COMPONENT)) {
				font_r->setDimensions(w, h);
			}
		}
		this->ortho = gore::matrix::calculateOrtho(w, h, this->window_width, this->window_height);
		this->window_width = w;
		this->window_height = h;
		if (func) {
			func(w, h);
		}
	};
	wind->setWindowResize(f);
}

void gore::g_engine_2d::setRendererViewportMask (uint32_t mask) {
	this->maintainRendererViewport = mask;
}

void gore::g_engine_2d::setMaintainViewport(bool maintain) {
	wind->setMaintainViewport(maintain);
}

void gore::g_engine_2d::setWindowTitle (std::string title) {
	this->wind->setWindowTitle(title);
}

void gore::g_engine_2d::updateView (float camera_x, float camera_y, float zoom) {
	if (component_mask & USE_VIEW_MATRICE) {
		if (img_r) {
			img_r->updateView(camera_x, camera_y, zoom);
		}
		if (gray_r) {
			gray_r->updateView(camera_x, camera_y, zoom);
		}
		if (triangle_r && line_r && point_r) {
			triangle_r->updateView(camera_x, camera_y, zoom);
			line_r->updateView(camera_x, camera_y, zoom);
			point_r->updateView(camera_x, camera_y, zoom);
		}
		if (font_r) {
			font_r->updateView(camera_x, camera_y, zoom);
		}
		this->view = gore::matrix::calculate2DView(camera_x, camera_y, zoom);
	}
}

void gore::g_engine_2d::toggleFrameLimitActive () {
	this->limitframes = !this->limitframes;
}
void gore::g_engine_2d::setFrameLimit (uint32_t fps) {
	this->fps = fps;
	this->frame_time_limit = 1000.0f / this->fps;
}