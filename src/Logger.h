#include <iostream>

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>

std::ostream& operator <<(std::ostream& os, const glm::vec3& v)
{
	os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
	return os;
}