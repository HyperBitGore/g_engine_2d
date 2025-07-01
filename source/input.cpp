#include "backend.hpp"
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>



void Input::setLastState() {
	memcpy(last_state, keys, keys_size);
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
				if (keys[i] & (1 << j)) {
					KeySym sym = XkbKeycodeToKeysym(display, i * 8 + j, 0, 0);
					keys[sym] = 1;
				}
			}
		}
	}
	#endif
	return success;
}


bool Input::GetKeyDown(uint32_t key) {
	getState();
	short t = key;
	#if defined (_WIN32)
	if (key >= 97 && key <= 122) {
		t = VkKeyScanEx(key, layout);
	}
	#endif
	if (keys[t]) {
		return true;
	}
	return false;
}
bool Input::GetKeyReleased(uint32_t key) {
	short t = key;
	#if defined (_WIN32)
	if (key >= 97 && key <= 122) {
		t = VkKeyScanEx(key, layout);
	}
	#endif
	if ((last_state[t]) != (keys[t]) && (keys[t]) != 0) {
		getState();
		return true;
	}
	getState();
	return false;
}

