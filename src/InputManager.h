#pragma once

#include <map>

#include "CrossWindow/CrossWindow.h"

enum class InputAxis
{
	MOUSE_X,
	MOUSE_Y
};

// Used to query the state of keypresses.
// Notify methods are called by the main event loop, and should not be used directly.
// Right now it only manages the state of each key.
// In the future will do action name mapping
class InputManager
{
public:
	// Returns true if the given keyboard key is currently down
	bool isDown(xwin::Key key);

	// Returns true if the given mouse button is currently down
	bool isDown(xwin::MouseInput mouseButton);

	float getAxis(const InputAxis& inputAxis);

	// Called each frame before the start of the windows message loop.
	void notifyEventLoopBefore();

	// Called by event loop when the window loses focus. Clears internal state.
	void notifyLostFocus();

	// Called by event loop when a hardware key press is detected.
	void notifyKeyStateChange(xwin::Key key, xwin::ButtonState state);

	// Called by event loop when a mouse button state change is detected.
	void notifyMouseButtonChange(xwin::MouseInput button, xwin::ButtonState state);

	// Called by event loop when raw mouse movement occurs
	void notifyMouseRawInput(int xDelta, int yDelta);

private:
	std::map<xwin::Key, bool> keyStates;
	std::map<xwin::MouseInput, bool> mouseStates;
	std::map<InputAxis, float> axisValues;
};