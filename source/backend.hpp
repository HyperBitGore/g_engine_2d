#include "gl_defines.hpp"


//added customizability and clean up
class Window {
private:
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	LPCSTR class_name;
	int height;
	int width;
public:
	Window(LPCSTR title, LPCSTR CLASS_NAME, int h, int w, int x, int y);
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
		layout = LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);
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