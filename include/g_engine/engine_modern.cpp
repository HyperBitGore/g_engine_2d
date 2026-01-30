#include "g_engine_2d.hpp"
#include <cstdint>

bool gore::g_engine_2d::getKeyDown(uint32_t key) {
	return in->GetKeyDown(key);
}
bool gore::g_engine_2d::getKeyReleased(uint32_t key) {
	return in->GetKeyReleased(key);
}

void gore::g_engine_2d::updateInputState(){
	in->updateState();
}

gore::vec2 gore::g_engine_2d::getMousePos() {
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
	return p;
}

//returns the frame time in seconds
double gore::g_engine_2d::getDelta() {
	clock_t d = delta;
	delta = 0;
	return d / (double)CLOCKS_PER_SEC;
}
//returns number of frames in a second and the average frame time in milliseconds, every second. 
std::pair<double, double> gore::g_engine_2d::getFrames() {
	if (clockToMilliseconds(delta_f) > 1000.0) { //every second
		frameRate = (double)frames * 0.5 + frameRate * 0.5; //more stable
		frames = 0;
		delta_f -= CLOCKS_PER_SEC;
		averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);
		return { frameRate, averageFrameTimeMilliseconds };
	}
	return { frameRate, averageFrameTimeMilliseconds };
}


//utility type functions

bool gore::g_engine_2d::updateWindow() {
	wind->updateWindow();
	if (!wind->ProcessMessage()) {
		logger->log("Closing window");
		delete wind;
		return false;
	}
	begin_f = clock();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderFund();
	

	if (!wind->swapBuffers())
	{
		FatalError("Failed to swap OpenGL buffers!");
		return false;
	}
	end_f = clock();
	delta = end_f - begin_f;
	delta_f += end_f - begin_f;
	frames++;
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

void gore::g_engine_2d::toggleMouseHide() {
	wind->mouseHideToggle();	
}

uint32_t gore::g_engine_2d::getDPI() {
	return wind->getDPI();
}

void gore::g_engine_2d::setWindowResize(std::function<void(uint32_t, uint32_t)> func) {
	std::function<void(uint32_t, uint32_t)> f = [this, func](uint32_t w, uint32_t h) {
		if (prim_r && (maintainRendererViewport & PRIMITIVE_COMPONENT)) {
			prim_r->setDimensions(w, h);
		}
		if (img_r && (maintainRendererViewport & IMAGE_COMPONENT)) {
			img_r->setDimensions(w, h);
		}
		if (gray_r && (maintainRendererViewport & GRAYSCALE_COMPONENT)) {
			gray_r->setDimensions(w, h);
		}
		if (font_renderer && (maintainRendererViewport & FONT_COMPONENT)) {
			font_renderer->setDimensions(w, h);
		}
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
