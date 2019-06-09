#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

glm::mat4x4 Camera::getViewProjectionMatrix()
{
	// note: not cached because this only happens once per frame
	auto forward = getForward();
	auto up = getUp();

	auto position = this->getPosition();
	auto viewM = glm::lookAt(position, position + forward, up);
	auto projectionM = glm::perspective(glm::radians(fov), aspectRatio, nearZ, farZ);
	return projectionM * viewM;
}
