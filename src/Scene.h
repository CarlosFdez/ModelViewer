#pragma once

#include <vector>
#include <memory>

#include "Assets.h"

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>

class Scene;
class SceneObject;

typedef std::shared_ptr<Scene> ScenePtr;
typedef std::shared_ptr<SceneObject> SceneObjectPtr;

// Represents a scene, that can contain multiple scene objects
class Scene
{
public:
	SceneObjectPtr createObject(const MeshResourcePtr& mesh);

	std::vector<SceneObjectPtr>::const_iterator begin() const
	{ 
		return this->objects.begin();
	}

	std::vector<SceneObjectPtr>::const_iterator end() const
	{
		return this->objects.end();
	}
private:
	std::vector<SceneObjectPtr> objects;
};

// A single object in a scene. Must be created via Scene::createObject.
class SceneObject
{
public:
	explicit SceneObject() = default;

	MeshResourcePtr mesh = nullptr;

	// Get the camera's position in the world
	glm::vec3 getPosition() const { return worldPosition; }

	// Sets the objects's position
	void setPosition(const glm::vec3& position);

	// Sets the object's location
	void setPosition(float x, float y, float z)
	{
		setPosition({ x, y, z });
	}

	// Moves the object by a certain amount
	void move(const glm::vec3& moveDelta);

	void setScale(float x, float y, float z);

	void setScale(float s)
	{
		this->setScale(s, s, s);
	}

	// Sets the rotation values (in degrees)
	void setRotation(const glm::vec3& eulerAngles);

	// sets the rotation values (in degrees)
	void setRotation(float x, float y, float z)
	{
		this->setRotation({ x, y, z });
	}

	void addRotation(const glm::vec3& eulerAngles);

	void addRotation(int x, int y, int z);

	void rotateAround(const glm::vec3& axisOfRotation, float angleDegrees);

	// Returns the current rotation in euler angles (in degrees)
	glm::vec3 getRotation() const;

	// Get the camera's forward direction
	glm::vec3 getForward() const
	{
		glm::vec3 forward = { 0.0f, 0.0f, 1.0f };
		return rotation * forward;
	}

	// Get the camera's up direction.
	glm::vec3 getUp() const
	{
		glm::vec3 up = { 0.0f, 1.0f, 0.0f };
		return rotation * up;
	}

	glm::vec3 getRight() const
	{
		glm::vec3 right = { 1.0f, 0.0f, 0.0f };
		return rotation * right;
	}

	// todo: rotation

	const glm::mat4x4& getModelMatrix();

private:
	// private function to apply a rotation as a quaternion
	// could be made public in the future
	void applyRotation(const glm::quat& q);

	// The model matrix. Defaults to identity
	glm::mat4x4 modelMatrix = glm::mat4x4(1.0f);

	// Whether the model matrix needs to be recreated
	bool dirty = false;

	glm::vec3 worldPosition = { 0, 0, 0 };
	glm::vec3 scaling = { 1, 1, 1 };
	glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
};
