#include "InputManager.h"
#include "InputManager.h"
#include "InputManager.h"

// helper to return a map's value if it exists, or some default value.
// comes from https://stackoverflow.com/a/2333816
template <template<class, class, class...> class C, typename K, typename V, typename... Args>
V getOrDefault(const C<K, V, Args...>& m, const K& key, const V& defVal)
{
	typename C<K, V, Args...>::const_iterator it = m.find(key);
	if (it == m.end())
	{
		return defVal;
	}
	return it->second;
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

bool InputManager::isDown(xwin::MouseInput mouseButton)
{
	auto iter = mouseStates.find(mouseButton);
	if (iter == mouseStates.end())
	{
		return false;
	}

	return iter->second;
}

float InputManager::getAxis(const InputAxis& inputAxis)
{
	return getOrDefault(this->axisValues, inputAxis, 0.0f);
}

void InputManager::notifyUpdateFinished()
{
	// axis values are cleared before each frame
	axisValues.clear();
}

void InputManager::notifyLostFocus()
{
	keyStates.clear();
	mouseStates.clear();
	axisValues.clear();
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

void InputManager::notifyMouseButtonChange(xwin::MouseInput button, xwin::ButtonState state)
{
	if (state == xwin::ButtonState::Pressed)
	{
		mouseStates[button] = true;
	}
	else if (state == xwin::ButtonState::Released)
	{
		mouseStates.erase(button);
	}
}

void InputManager::notifyMouseRawInput(int xDelta, int yDelta)
{
	if (xDelta != 0)
	{
		auto curValue = getOrDefault(axisValues, InputAxis::MOUSE_X, 0.0f);
		axisValues[InputAxis::MOUSE_X] = curValue + float(xDelta);
	}
	if (yDelta != 0)
	{
		auto curValue = getOrDefault(axisValues, InputAxis::MOUSE_Y, 0.0f);
		axisValues[InputAxis::MOUSE_Y] = curValue + float(yDelta);
	}
}
