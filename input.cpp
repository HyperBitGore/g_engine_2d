#include "g_engine_2d.h"



void Input::setLastState() {
	for (int i = 0; i < 256; i++) {
		last_state[i] = keys[i];
	}
}
//input functions
//probably switch to GetKeyboardState, so easier to use getKeyReleased
void Input::getState() {
	setLastState();
	bool fail = GetKeyboardState((PBYTE)keys);
}


bool Input::GetKeyDown(char key) {
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
bool Input::getKeyReleased(char key) {
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