#pragma once

#include <map>

#include "CrossWindow/CrossWindow.h"

enum class InputAxis
{
	MOUSE_X,
	MOUSE_Y
};

// Used to query the state of key and mouse inputs.
// Notify methods are called by the main event loop, and should not be used directly.
class InputManager
{
public:
	// Returns true if the given keyboard key is currently down
	bool isDown(xwin::Key key);

	// Returns true if the given mouse button is currently down
	bool isDown(xwin::MouseInput mouseButton);

	// Returns the value for a particular given axis.
	// For Mouse values, it returns the number of pixels travelled during the current frame.
	float getAxis(const InputAxis& inputAxis);

	// Called each frame at the very end to clean up data to prepare for the next update
	void notifyUpdateFinished();

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