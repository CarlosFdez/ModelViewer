#pragma once

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// Class that represents a camera in the world.
// Used to create a view matrix to use when the object is rendered.
class Camera
{
public:
	// Sets the camera vertical fov in degrees
	void setFov(float fov)
	{
		this->fov = fov;
		dirty = true;
	}

	// Sets the camera's aspect ratio. Use when the rendering area is resized
	void setAspectRatio(float aspectRatio)
	{
		this->aspectRatio = aspectRatio;
		dirty = true;
	}

	// Sets the camera's near clip range and view distance
	void setClipRange(float nearZ, float farZ)
	{
		this->nearZ = nearZ;
		this->farZ = farZ;
		dirty = true;
	}

	// Sets the camera's position
	void setPosition(const glm::vec3& position)
	{
		this->position = position;
		dirty = true;
	}

	// Sets the camera's position
	void setPosition(float x, float y, float z)
	{
		setPosition({ x, y, z });
	}

	// Get the camera's position in the world
	glm::vec3 getPosition()
	{
		return position;
	}

	// Get the camera's forward direction
	glm::vec3 getForward()
	{
		return forward;
	}

	// Get the camera's up direction.
	glm::vec3 getUp()
	{
		return up;
	}

	// Returns the view projection matrix that can be used to modify all other objects to
	// be within range of the camera in the aspect ratio
	glm::mat4x4 getViewProjectionMatrix();

private:
	float fov = 45.0f;
	float aspectRatio = 1;
	float nearZ = 0.1f;
	float farZ = 50.0f;

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 forward = { 0.0f, 0.0f, 1.0f };
	glm::vec3 up = { 0.0f, 1.0f, 1.0f };

	bool dirty = false;
	glm::mat4x4 viewProjectionMatrix = glm::mat4x4(1.0f);
};