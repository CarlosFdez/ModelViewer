#pragma once

#include <map>

#include "CrossWindow/CrossWindow.h"

// Used to query the state of keypresses.
// Right now it only manages the state of each key.
// In the future will do action name mapping
class InputManager
{
public:
	void clear();

	bool isDown(xwin::Key key);

	// Call when a hardware key press is detected. Updates the internal state.
	void notifyKeyStateChange(xwin::Key key, xwin::ButtonState state);

private:
	std::map<xwin::Key, bool> keyStates;
};