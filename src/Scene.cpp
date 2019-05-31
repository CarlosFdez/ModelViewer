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

const glm::mat4x4& SceneObject::getModelMatrix()
{
	if (!dirty)
	{
		return modelMatrix;
	}

	//auto world_m = glm::translate(glm::mat4x4(1.0f), glm::vec3(0, -0.5f, 3.0f));
	//world_m = glm::scale(world_m, glm::vec3(0.3f));

	// pass-throughs are applied in reverse order. The bottom is applied first
	// we want to translate at the very end, so it goes at the top
	// scaling must happen first, or future operations would be scaled. Goes at the bottom
	modelMatrix = glm::translate(glm::mat4x4(1.0f), this->worldPosition);
	// todo: rotation...which is a bit more complicated
	modelMatrix = glm::scale(modelMatrix, this->scaling);

	auto translation = glm::translate(glm::mat4x4(1.0f), this->worldPosition);
	auto scale = glm::scale(glm::mat4x4(1.0f), this->scaling);
	modelMatrix = translation * scale;

	dirty = false;
	return modelMatrix;
}

