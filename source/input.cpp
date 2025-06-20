#include "backend.hpp"



void Input::setLastState() {
	for (int i = 0; i < 256; i++) {
		last_state[i] = keys[i];
	}
}
//input functions
//probably switch to GetKeyboardState, so easier to use getKeyReleased
void Input::getState() {
	setLastState();
	#if defined(_WIN32)
	bool fail = GetKeyboardState((PBYTE)keys);
	#endif
	#if defined(__unix)
	bool fail = XkbGetState(display, XkbUseCoreKbd, &state);
	#endif
}


bool Input::GetKeyDown(uint32_t key) {
	getState();
	short t = key;
	if (key >= 97 && key <= 122) {
		t = VkKeyScanEx(key, layout);
	}
	if ((keys[t] >> 15) == -1) {
		return true;
	}
	return false;
}
bool Input::GetKeyReleased(uint32_t key) {
	short t = key;
	if (key >= 97 && key <= 122) {
		t = VkKeyScanEx(key, layout);
	}
	if ((last_state[t] >> 15) != (keys[t] >> 15) && (keys[t] >> 15) != -1) {
		getState();
		return true;
	}
	getState();
	return false;
}

