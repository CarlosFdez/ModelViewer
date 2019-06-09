#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>

// helper to facilitate wraparound
template<typename T>
T wraparound(T value, T minValue, T maxValue)
{
	if (value < minV)
}

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

void SceneObject::setScale(float x, float y, float z)
{
	scaling = { x, y, z };
	dirty = true;
}

void SceneObject::setRotation(const glm::vec3& eulerAngles)
{
	// todo: Ensure rotates around Z, then X, then Y.
	auto angles = glm::mod(eulerAngles, 360.0f);
	this->rotation = glm::quat(glm::radians(eulerAngles));
	dirty = true;
}

void SceneObject::addRotation(const glm::vec3& eulerAngles)
{
	auto rotation = glm::quat(glm::radians(eulerAngles));
	this->applyRotation(rotation);
}

void SceneObject::addRotation(int x, int y, int z)
{
	addRotation({ x, y, z });
}

void SceneObject::rotateAround(const glm::vec3& axisOfRotation, float angleDegrees)
{
	auto r = glm::angleAxis(glm::radians(angleDegrees), axisOfRotation);
	applyRotation(r);
}

glm::vec3 SceneObject::getRotation() const
{
	return glm::degrees(glm::eulerAngles(this->rotation));
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

void SceneObject::applyRotation(const glm::quat& q)
{
	this->rotation = q * this->rotation;
	dirty = true;
}
