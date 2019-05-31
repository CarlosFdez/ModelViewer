#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective


glm::mat4x4 Camera::getViewProjectionMatrix()
{
	if (dirty)
	{
		auto viewM = glm::lookAt(position, position + forward, up);
		auto projectionM = glm::perspective(glm::radians(fov), aspectRatio, nearZ, farZ);
		viewProjectionMatrix = projectionM * viewM;

		dirty = false;
	}

	return viewProjectionMatrix;
}
