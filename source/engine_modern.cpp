#include "g_engine_2d.hpp"

bool EngineNewGL::getKeyDown(uint32_t key) {
	return in->GetKeyDown(key);
}
bool EngineNewGL::getKeyReleased(uint32_t key) {
	return in->GetKeyReleased(key);
}

void EngineNewGL::updateInputState(){
	in->updateState();
}

vec2 EngineNewGL::getMousePos() {
	vec2 p;
	#if defined(_WIN32)
	LPPOINT po = new tagPOINT;
	GetCursorPos(po);
	ScreenToClient(wind->getRawWindow(), po);
	p.x = (float)po->x;
	p.y = (float)po->y;
	#endif
	#if defined(__unix__)
	int x, y, win_x, win_y;
	Window root_return, child_return;
	unsigned int mask_return;
	XQueryPointer(display, wind->getRawWindow(),
		&root_return, &child_return,
		&x, &y,       // Root (global) coords
		&win_x, &win_y, // Window-relative coords
		&mask_return);
	p.x = (float)win_x;
	p.y = (float)win_y;
	#endif

	//gotta translate the y axis for my coord system
	p.y = p.y - wind->getHeight();
	p.y = std::abs(p.y);

	#if defined(_WIN32)
	delete po;
	#endif
	return p;
}

//returns the frame time in seconds
double EngineNewGL::getDelta() {
	clock_t d = delta;
	delta = 0;
	return d / (double)CLOCKS_PER_SEC;
}
//returns number of frames in a second and the average frame time in milliseconds, every second. 
std::pair<double, double> EngineNewGL::getFrames() {
	if (clockToMilliseconds(delta_f) > 1000.0) { //every second
		frameRate = (double)frames * 0.5 + frameRate * 0.5; //more stable
		//std::cout << "Frames: " << frameRate << "\n";
		frames = 0;
		delta_f -= CLOCKS_PER_SEC;
		averageFrameTimeMilliseconds = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);
		//std::cout << "CPU time was:" << averageFrameTimeMilliseconds << std::endl;
		return { frameRate, averageFrameTimeMilliseconds };
	}
	return { frameRate, averageFrameTimeMilliseconds };
}


//utility type functions

bool EngineNewGL::updateWindow() {
	wind->updateWindow();
	//UpdateWindow(wind->getHwnd());
	if (!wind->ProcessMessage()) {
		std::cout << "Closing window\n";
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
	}
	end_f = clock();
	delta = end_f - begin_f;
	delta_f += end_f - begin_f;
	frames++;
	return true;
}


void EngineNewGL::enable(GLenum en){
	glEnable(en);
}
void EngineNewGL::disable(GLenum en){
	glDisable(en);
}

void EngineNewGL::setWindowFullscreen(bool mode) {
	if (mode) {
		wind->setWindowFullscreen();
	}
}