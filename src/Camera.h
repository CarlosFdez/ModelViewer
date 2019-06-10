#pragma once

#include "Scene.h"

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// Class that represents a camera in the world.
// Used to create a view matrix to use when the object is rendered.
class Camera : public SceneObject
{
public:
	// Sets the camera vertical fov in degrees
	void setFov(float fov)
	{
		this->fov = fov;
	}

	// Sets the camera's aspect ratio. Use when the rendering area is resized
	void setAspectRatio(float aspectRatio)
	{
		this->aspectRatio = aspectRatio;
	}

	// Sets the camera's aspect ratio. Use when the rendering area is resized
	void setAspectRatio(unsigned width, unsigned height)
	{
		setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	}

	// Sets the camera's near clip range and view distance
	void setClipRange(float nearZ, float farZ)
	{
		this->nearZ = nearZ;
		this->farZ = farZ;
	}

	// Returns the view projection matrix that can be used to modify all other objects to
	// be within range of the camera in the aspect ratio.
	// Because of how often a camera is expected to update, this value is not cached.
	// Store the result in a variable before rendering the frame.
	glm::mat4x4 getViewProjectionMatrix();

private:
	float fov = 45.0f;
	float aspectRatio = 1;
	float nearZ = 0.1f;
	float farZ = 50.0f;
};