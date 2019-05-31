#pragma once

#include <memory>

#include <glm/vec3.hpp>

class Vertex
{
public:
	Vertex() : Vertex({ 0, 0, 0 }, { 0, 0, 0 }) {}
	Vertex(glm::vec3 position) : position{ position }, color{ 0, 0, 0 } {}
	Vertex(glm::vec3 position, glm::vec3 color) : position{ position }, color{ color } {}

	glm::vec3 position;
	glm::vec3 color;
};

// Defines a resource containing a mesh.
// Contains the vertices, indices, and the primitives used to render it.
struct MeshResource
{
	MeshResource() = default;

	// Default move constructor
	MeshResource(MeshResource&& other) = default;

	// CPU accessible vertices and indices
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	// Arbitrary primative data. todo: accept a joined object that is rendering engine independant
	std::shared_ptr<void> primitiveBuffers;
};

typedef std::shared_ptr<MeshResource> MeshResourcePtr;