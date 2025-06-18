#include "gl_defines.hpp"

#if defined(_WIN32)
#define RAW_WINDOW HWND
#endif
#if defined(__unix__)
#define RAW_WINDOW Window
#endif

//added customizability and clean up
class g_window {
private:
	#if defined(_WIN32)
	HINSTANCE m_hinstance;
	#endif
	RAW_WINDOW m_hwnd;
	const char* class_name;
	int height;
	int width;
public:
	g_window(const char* title, const char* CLASS_NAME, int h, int w, int x, int y);
	g_window(const Window&) = delete;
	g_window& operator =(const Window&) = delete;
	~g_window();
	bool ProcessMessage();
	int getWidth();
	int getHeight();
	RAW_WINDOW getRawWindow();
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