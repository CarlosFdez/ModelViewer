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
	// creates a scene object, registers it, and returns it.
	// Todo: Allow a <T> param to specify a class type. Make MeshResourcePtr optional.
	SceneObjectPtr createObject(const MeshResourcePtr& mesh);

	// Returns iterator to iterate over the list of registered objects.
	std::vector<SceneObjectPtr>::const_iterator begin() const
	{ 
		return this->objects.begin();
	}

	std::vector<SceneObjectPtr>::const_iterator end() const
	{
		return this->objects.end();
	}
private:
	// list of stored scene objects
	std::vector<SceneObjectPtr> objects;
};

// Represents an object in a scene. Must be created via Scene::createObject.
class SceneObject
{
public:
	explicit SceneObject() = default;

	MeshResourcePtr mesh = nullptr;

	// Get the camera's position in the world
	glm::vec3 getPosition() const { return worldPosition; }

	// Sets the objects's position
	void setPosition(const glm::vec3& position);

	// Sets the object's position
	void setPosition(float x, float y, float z)
	{
		setPosition({ x, y, z });
	}

	// Moves the object's position by a certain amount
	void move(const glm::vec3& moveDelta);

	// Sets the scaling for each object per axis. Applied before rotation
	void setScale(float x, float y, float z);

	// Sets the object's scaling to a value. Applied before rotation
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

	// Further rotates this object by an additional amount.
	// If the object hasn't been rotated yet, it is equal to setRotation().
	void addRotation(const glm::vec3& eulerAngles);

	// Further rotates this object by an additional amount.
	// If the object hasn't been rotated yet, it is equal to setRotation().
	void addRotation(int x, int y, int z);

	// Rotates this object around an axis of rotation by a certain number of degrees.
	// Rotations around an axis follow a left-hand rule.
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

	// Gets the camera's right direction.
	glm::vec3 getRight() const
	{
		glm::vec3 right = { 1.0f, 0.0f, 0.0f };
		return rotation * right;
	}

	// Returns the model matrix used to position this scene object into the world.
	// Given to the shader to move the vertices during the draw call.
	const glm::mat4x4& getModelMatrix();

private:
	// private function to apply a rotation as a quaternion
	// could be made public in the future
	void applyRotation(const glm::quat& q);

	// The model matrix's cached value. Defaults to identity
	glm::mat4x4 modelMatrix = glm::mat4x4(1.0f);

	// Whether the model matrix cached value is dirty and needs to be recreated
	bool dirty = false;

	glm::vec3 worldPosition = { 0, 0, 0 };
	glm::vec3 scaling = { 1, 1, 1 };
	glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
};
