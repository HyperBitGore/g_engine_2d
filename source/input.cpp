#include "backend.hpp"
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>



void Input::setLastState() {
	memcpy(last_state, keys, keys_size);
	last_lstate = cur_lstate;
	last_rstate = cur_rstate;
	last_mstate = cur_mstate;
}
//input functions
//probably switch to GetKeyboardState, so easier to use getKeyReleased
bool Input::getState() {
	setLastState();
	#if defined(_WIN32)
	bool success = GetKeyboardState((PBYTE)keys);
	#endif
	#if defined(__unix)
	bool success = XQueryKeymap(display, raw_keys);
	if (success) {
		for (size_t i = 0; i < 32; i++) {
			for (size_t j = 0; j < 8; j++) {
				KeySym sym = XkbKeycodeToKeysym(display, i * 8 + j, 0, 0);
				if (raw_keys[i] & (1 << j) && sym < keys_size) {
					keys[sym] = 1;
				} else if (sym < keys_size){
					keys[sym] = 0;
				}
			}
		}
	}
	Window returned_root, returned_child;
	int root_x, root_y, win_x, win_y;
	unsigned int mask_return;
	success = XQueryPointer(display, wind,
								&returned_root, &returned_child,
								&root_x, &root_y, &win_x, &win_y,
								&mask_return);
	cur_lstate = (mask_return & Button1Mask) >> 8;
	cur_rstate = (mask_return & Button2Mask) >> 9;
	cur_rstate = (mask_return & Button3Mask) >> 10;
	#endif
	return success;
}


bool Input::GetKeyDown(uint32_t key) {
	getState();
	#if defined (_WIN32)
	if (key >= 97 && key <= 122) {
		t = VkKeyScanEx(key, layout);
	}
	#endif
	switch (key) {
		case g_MouseLeft:
			return cur_lstate;
		case g_MouseRight:
			return cur_rstate;
		case g_MouseMiddle:
			return cur_mstate;
	}
	if (keys[key]) {
		return true;
	}
	return false;
}
bool Input::GetKeyReleased(uint32_t key) {
	#if defined (_WIN32)
	if (key >= 97 && key <= 122) {
		t = VkKeyScanEx(key, layout);
	}
	#endif
	switch (key) {
		case g_MouseLeft:
			return cur_lstate - last_lstate == -1;
		case g_MouseRight:
			return cur_rstate - last_rstate == -1;
		case g_MouseMiddle:
			return cur_mstate - last_mstate == -1;
	}
	if ((last_state[key]) != (keys[key]) && (keys[key]) != 0) {
		getState();
		return true;
	}
	getState();
	return false;
}

