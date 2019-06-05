#include "InputManager.h"
#include "InputManager.h"
#include "InputManager.h"

void InputManager::clear()
{
	keyStates.clear();
}

bool InputManager::isDown(xwin::Key key)
{
	auto iter = keyStates.find(key);
	if (iter == keyStates.end())
	{
		return false;
	}

	return iter->second;
}

void InputManager::notifyKeyStateChange(xwin::Key key, xwin::ButtonState state)
{
	if (state == xwin::ButtonState::Pressed)
	{
		keyStates[key] = true;
	}
	else if (state == xwin::ButtonState::Released)
	{
		keyStates.erase(key);
	}
}
