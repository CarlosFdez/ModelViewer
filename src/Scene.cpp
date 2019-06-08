#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

SceneObjectPtr Scene::createObject(const MeshResourcePtr& mesh)
{
	SceneObjectPtr newObject(new SceneObject());

	if (mesh)
	{
		newObject->mesh = mesh;
	}

	this->objects.push_back(newObject);
	return newObject;
}

void SceneObject::setPosition(const glm::vec3& position)
{
	worldPosition = position;
	dirty = true;
}

void SceneObject::move(const glm::vec3& moveDelta)
{
	setPosition(this->worldPosition + moveDelta);
}

// set the rotation as a set of euler angles. 
void SceneObject::setRotation(const glm::vec3& eulerAngles)
{
	// todo: Ensure rotates around Z, then X, then Y.
	auto angleXRad = glm::radians(eulerAngles.x);
	auto angleYRad = glm::radians(eulerAngles.y);
	auto angleZRad = glm::radians(eulerAngles.z);
	glm::vec3 eulerAngleRadians(angleXRad, angleYRad, angleZRad);
	this->rotation = glm::quat(eulerAngleRadians);
}

const glm::mat4x4& SceneObject::getModelMatrix()
{
	if (!dirty)
	{
		return modelMatrix;
	}

	//auto world_m = glm::translate(glm::mat4x4(1.0f), glm::vec3(0, -0.5f, 3.0f));
	//world_m = glm::scale(world_m, glm::vec3(0.3f));

	auto translation = glm::translate(glm::mat4x4(1.0f), this->worldPosition);
	auto rotate = glm::toMat4(this->rotation);
	auto scale = glm::scale(glm::mat4x4(1.0f), this->scaling);
	modelMatrix = translation * rotate * scale;

	dirty = false;
	return modelMatrix;
}

