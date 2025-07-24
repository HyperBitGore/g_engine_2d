#include "gl_defines.hpp"
#include <cstdint>
#include <iostream>
#include <cstring>

#if defined(_WIN32)
#define RAW_WINDOW HWND
#define RAW_DISPLAY HINSTANCE
#define PREVENT_PACKING_STRUCT __pragma(pack(push, 1)) struct
#define END_PACKING_STRUCT __pragma(pack(pop))
#define g_A 'A'
#define g_B 'B'
#define g_C 'C'
#define g_D 'D'
#define g_E 'E'
#define g_F 'F'
#define g_G 'G'
#define g_H 'H'
#define g_I 'I'
#define g_J 'J'
#define g_K 'K'
#define g_L 'L'
#define g_M 'M'
#define g_N 'N'
#define g_O 'O'
#define g_P 'P'
#define g_Q 'Q'
#define g_R 'R'
#define g_S 'S'
#define g_T 'T'
#define g_U 'U'
#define g_V 'V'
#define g_W 'W'
#define g_X 'X'
#define g_Y 'Y'
#define g_Z 'Z'

// Lowercase alphabetic keys (Shift is used for lowercase)
#define g_a 'A'
#define g_b 'B'
#define g_c 'C'
#define g_d 'D'
#define g_e 'E'
#define g_f 'F'
#define g_g 'G'
#define g_h 'H'
#define g_i 'I'
#define g_j 'J'
#define g_k 'K'
#define g_l 'L'
#define g_m 'M'
#define g_n 'N'
#define g_o 'O'
#define g_p 'P'
#define g_q 'Q'
#define g_r 'R'
#define g_s 'S'
#define g_t 'T'
#define g_u 'U'
#define g_v 'V'
#define g_w 'W'
#define g_x 'X'
#define g_y 'Y'
#define g_z 'Z'

#define g_1 '1'
#define g_2 '2'
#define g_3 '3'
#define g_4 '4'
#define g_5 '5'
#define g_6 '6'
#define g_7 '7'
#define g_8 '8'
#define g_9 '9'
#define g_0 '0'

#define g_Comma VK_OEM_COMMA
#define g_Period VK_OEM_PERIOD
#define g_Slash VK_OEM_2
#define g_Semicolon VK_OEM_1
#define g_Apostrophe VK_OEM_7
#define g_Equal VK_OEM_PLUS
#define g_Minus VK_OEM_MINUS
#define g_Tilde VK_OEM_3

#define g_Space VK_SPACE
#define g_Tab VK_TAB
#define g_Return VK_RETURN
#define g_Backspace VK_BACK
#define g_Escape VK_ESCAPE
#define g_CapsLock VK_CAPITAL
#define g_Insert VK_INSERT
#define g_Delete VK_DELETE
#define g_Home VK_HOME
#define g_End VK_END
#define g_PageUp VK_PRIOR
#define g_PageDown VK_NEXT

#define g_LShift VK_LSHIFT
#define g_RShift VK_RSHIFT
#define g_LControl VK_LCONTROL
#define g_RControl VK_RCONTROL
#define g_LAlt VK_LMENU
#define g_RAlt VK_RMENU
#define g_CapsLock VK_CAPITAL
#define g_NumLock VK_NUMLOCK
#define g_ScrollLock VK_SCROLL

#define g_F1 VK_F1
#define g_F2 VK_F2
#define g_F3 VK_F3
#define g_F4 VK_F4
#define g_F5 VK_F5
#define g_F6 VK_F6
#define g_F7 VK_F7
#define g_F8 VK_F8
#define g_F9 VK_F9
#define g_F10 VK_F10
#define g_F11 VK_F11
#define g_F12 VK_F12

#define g_UpArrow VK_UP
#define g_DownArrow VK_DOWN
#define g_LeftArrow VK_LEFT
#define g_RightArrow VK_RIGHT

#define g_KP_1 VK_NUMPAD1
#define g_KP_2 VK_NUMPAD2
#define g_KP_3 VK_NUMPAD3
#define g_KP_4 VK_NUMPAD4
#define g_KP_5 VK_NUMPAD5
#define g_KP_6 VK_NUMPAD6
#define g_KP_7 VK_NUMPAD7
#define g_KP_8 VK_NUMPAD8
#define g_KP_9 VK_NUMPAD9
#define g_KP_0 VK_NUMPAD0
#define g_KP_Enter VK_RETURN
#define g_KP_Plus VK_ADD
#define g_KP_Minus VK_SUBTRACT
#define g_KP_Multiply VK_MULTIPLY
#define g_KP_Divide VK_DIVIDE
#define g_KP_Dot VK_DECIMAL

#define g_MouseLeft VK_LBUTTON
#define g_MouseRight VK_RBUTTON
#define g_MouseMiddle VK_MBUTTON
#endif
#if defined(__unix__)
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XKBstr.h>
#define RAW_WINDOW Window
#define RAW_DISPLAY Display*
#define PREVENT_PACKING_STRUCT struct __attribute__((__packed__))
#define END_PACKING_STRUCT 
#define g_A XK_A
#define g_B XK_B
#define g_C XK_C
#define g_D XK_D
#define g_E XK_E
#define g_F XK_F
#define g_G XK_G
#define g_H XK_H
#define g_I XK_I
#define g_J XK_J
#define g_K XK_K
#define g_L XK_L
#define g_M XK_M
#define g_N XK_N
#define g_O XK_O
#define g_P XK_P
#define g_Q XK_Q
#define g_R XK_R
#define g_S XK_S
#define g_T XK_T
#define g_U XK_U
#define g_V XK_V
#define g_W XK_W
#define g_X XK_X
#define g_Y XK_Y
#define g_Z XK_Z

#define g_a XK_a
#define g_b XK_b
#define g_c XK_c
#define g_d XK_d
#define g_e XK_e
#define g_f XK_f
#define g_g XK_g
#define g_h XK_h
#define g_i XK_i
#define g_j XK_j
#define g_k XK_k
#define g_l XK_l
#define g_m XK_m
#define g_n XK_n
#define g_o XK_o
#define g_p XK_p
#define g_q XK_q
#define g_r XK_r
#define g_s XK_s
#define g_t XK_t
#define g_u XK_u
#define g_v XK_v
#define g_w XK_w
#define g_x XK_x
#define g_y XK_y
#define g_z XK_z

#define g_1 XK_1
#define g_2 XK_2
#define g_3 XK_3
#define g_4 XK_4
#define g_5 XK_5
#define g_6 XK_6
#define g_7 XK_7
#define g_8 XK_8
#define g_9 XK_9
#define g_0 XK_0

#define g_Comma XK_comma
#define g_Period XK_period
#define g_Slash XK_slash
#define g_Semicolon XK_semicolon
#define g_Apostrophe XK_apostrophe
#define g_Equal XK_equal
#define g_Minus XK_minus
#define g_Tilde XK_grave

#define g_Space XK_space
#define g_Tab XK_Tab
#define g_Return XK_Return
#define g_Backspace XK_BackSpace
#define g_Escape XK_Escape
#define g_CapsLock XK_Caps_Lock
#define g_Insert XK_Insert
#define g_Delete XK_Delete
#define g_Home XK_Home
#define g_End XK_End
#define g_PageUp XK_Page_Up
#define g_PageDown XK_Page_Down

#define g_LShift XK_Shift_L
#define g_RShift XK_Shift_R
#define g_LControl XK_Control_L
#define g_RControl XK_Control_R
#define g_LAlt XK_Alt_L
#define g_RAlt XK_Alt_R
#define g_CapsLock XK_Caps_Lock
#define g_NumLock XK_Num_Lock
#define g_ScrollLock XK_Scroll_Lock

#define g_F1 XK_F1
#define g_F2 XK_F2
#define g_F3 XK_F3
#define g_F4 XK_F4
#define g_F5 XK_F5
#define g_F6 XK_F6
#define g_F7 XK_F7
#define g_F8 XK_F8
#define g_F9 XK_F9
#define g_F10 XK_F10
#define g_F11 XK_F11
#define g_F12 XK_F12

#define g_UpArrow XK_Up
#define g_DownArrow XK_Down
#define g_LeftArrow XK_Left
#define g_RightArrow XK_Right

#define g_KP_1 XK_KP_1
#define g_KP_2 XK_KP_2
#define g_KP_3 XK_KP_3
#define g_KP_4 XK_KP_4
#define g_KP_5 XK_KP_5
#define g_KP_6 XK_KP_6
#define g_KP_7 XK_KP_7
#define g_KP_8 XK_KP_8
#define g_KP_9 XK_KP_9
#define g_KP_0 XK_KP_0
#define g_KP_Enter XK_KP_Enter
#define g_KP_Plus XK_KP_Add
#define g_KP_Minus XK_KP_Subtract
#define g_KP_Multiply XK_KP_Multiply
#define g_KP_Divide XK_KP_Divide
#define g_KP_Dot XK_KP_Decimal

#define g_MouseLeft 0x1000ffff
#define g_MouseRight 0x2000ffff
#define g_MouseMiddle 0x3000ffff
#endif

//added customizability and clean up
class g_window {
private:
	RAW_DISPLAY r_display;
	RAW_WINDOW m_hwnd;
	int height;
	int width;
public:
	g_window(const char* title, RAW_DISPLAY display, int h, int w, int x, int y);
	g_window(const g_window&) = delete;
	g_window& operator =(const g_window&) = delete;
	~g_window();
	bool ProcessMessage();
	int getWidth();
	int getHeight();
	RAW_WINDOW getRawWindow();
	RAW_DISPLAY getRawDisplay();
	bool updateWindow();
	bool swapBuffers();

};




class Input {
private:
	#if defined(_WIN32)
	HKL layout;
	char keys[256];
	char last_state[256];
	const uint32_t keys_size = 256;
	#endif
	#if defined(__unix__)
	XkbStateRec mod_state;
    XkbDescPtr xkb;
	XkbDescRec kb_state;
	char raw_keys[32];
	char keys[65535];
	char last_state[65535];
	const uint32_t keys_size = 65535;
	char cur_lstate = 0;
	char cur_rstate = 0;
	char cur_mstate = 0;
	char last_lstate = 0;
	char last_rstate = 0;
	char last_mstate = 0;
	#endif
	RAW_DISPLAY display;
	RAW_WINDOW wind;
	void setLastState();
	//input functions
	//probably switch to GetKeyboardState, so easier to use getKeyReleased
	bool getState();
public:
	Input(RAW_DISPLAY display, RAW_WINDOW wind) {
		#if defined(_WIN32)
		layout = LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);
		#endif
		#if defined(__unix__)
		xkb = XkbAllocKeyboard();
		if (!xkb) {
			std::cerr << "Failed to allocate keyboard\n";
		}
		#endif
		memset(keys, 0, keys_size);
		this->display = display;
		this->wind = wind;
	}
	Input(const Input&) = delete;
	Input& operator =(const Input&) = delete;
	~Input () {

	}
	void updateState();

	bool GetKeyDown(uint32_t key);
	bool GetKeyReleased(uint32_t key);

};