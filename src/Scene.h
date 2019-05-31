#pragma once

#include <vector>
#include <memory>

#include "Assets.h"


#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

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

	void setWorldPosition(float x, float y, float z)
	{
		worldPosition = { x, y, z };
		dirty = true;
	}

	void setScale(float x, float y, float z)
	{
		scaling = { x, y, z };
		dirty = true;
	}

	void setScale(float s)
	{
		this->setScale(s, s, s);
	}

	// todo: rotation

	const glm::mat4x4& getModelMatrix();

private:

	// The model matrix. Defaults to identity
	glm::mat4x4 modelMatrix = glm::mat4x4(1.0f);

	// Whether the model matrix needs to be recreated
	bool dirty = false;

	glm::vec3 worldPosition = { 0, 0, 0 };
	glm::vec3 scaling = { 1, 1, 1 };

};
