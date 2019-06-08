#include "ResourceManager.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <strstream>

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>

using namespace std;

void ResourceManager::initialize(DX11Interface* dx11)
{
	this->dx11 = dx11;
}

MeshResourcePtr ResourceManager::loadModel(const std::wstring& relativePath)
{
	// todo: CACHE
	// note: this assumes DX11, which means we negate the Z access and read faces backwards.
	// OBJ files assume right hand coordinate systems looking down negative Z.

	auto path = filesystem::current_path();
	path.append("assets\\models");
	path.append(relativePath);

	if (!filesystem::exists(path))
	{
		std::string msg("Could not find model file " + path.string());
		std::cout << msg << endl;
		throw std::exception(msg.c_str());
	}

	ifstream f(path);

	// store the information we're reading
	vector<Vertex> vertices;
	vector<unsigned> indices;

	string line;
	while (getline(f, line))
	{
		strstream s;
		s << line;

		string firstChar;
		s >> firstChar;

		if (firstChar == "v")
		{
			float x, y, z;
			s >> x >> y >> z;
			vertices.push_back(Vertex({ x, y, -z }, { 0.8, 0.8, 0.8 }));
		}
		else if (firstChar == "f")
		{
			// obj are counter clockwise, make clockwise and make them 0 indexed
			unsigned p1, p2, p3;
			//s >> p3 >> p2 >> p1;
			s >> p3 >> p2 >> p1;
			p1 -= 1;
			p2 -= 1;
			p3 -= 1;

			// calculate the normal, and add it to the normal of each vertex. 
			// We normalize in the end
			glm::vec3 v1 = vertices[p2].position - vertices[p1].position;
			glm::vec3 v2 = vertices[p3].position - vertices[p1].position;
			glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

			vertices[p1].normal += normal;
			vertices[p2].normal += normal;
			vertices[p3].normal += normal;

			indices.push_back(p1);
			indices.push_back(p2);
			indices.push_back(p3);
		}
	}

	// Create primitive buffers that will be used to render the mesh
	// todo: don't hardcode to dx11
	auto buffers = std::make_shared<D3D11PrimitiveBuffers>();
	buffers->vertexBuffer = dx11->createVertexBuffer(vertices);
	buffers->indexBuffer = dx11->createIndexBuffer(indices);

	// make vertex normals into unit vectors
	for (auto& vertex : vertices)
	{
		vertex.normal = glm::normalize(vertex.normal);
	}

	// Create the resource
	MeshResourcePtr resource(new MeshResource());
	resource->vertices = std::move(vertices);
	resource->indices = std::move(indices);
	resource->primitiveBuffers = buffers;

	return resource;
}