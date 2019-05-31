#include "ResourceManager.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <strstream>

#include <glm/vec3.hpp>

using namespace std;

void ResourceManager::initialize(DX11Interface* dx11)
{
	this->dx11 = dx11;
}

MeshResourcePtr ResourceManager::loadModel(const std::wstring& relativePath)
{
	// todo: CACHE

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
			vertices.push_back(Vertex({ x, y, z }, { 0.8, 0.8, 0.8 }));
		}
		else if (firstChar == "f")
		{
			unsigned p1, p2, p3;
			s >> p1 >> p2 >> p3;

			// obj are counter clockwise, make clockwise
			indices.push_back(p3 - 1);
			indices.push_back(p2 - 1);
			indices.push_back(p1 - 1);
		}
	}

	// Create primitive buffers that will be used to render the mesh
	// todo: don't hardcode to dx11
	auto buffers = std::make_shared<D3D11PrimitiveBuffers>();
	buffers->vertexBuffer = dx11->createVertexBuffer(vertices);
	buffers->indexBuffer = dx11->createIndexBuffer(indices);

	// Create the resource
	MeshResourcePtr resource(new MeshResource());
	resource->vertices = std::move(vertices);
	resource->indices = std::move(indices);
	resource->primitiveBuffers = buffers;

	return resource;
}