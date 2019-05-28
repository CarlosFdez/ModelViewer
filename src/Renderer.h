#pragma once

#include <vector>
#include <memory>
#include <wrl/client.h>

#include <glm/vec3.hpp>

#include "DX11Interface.h"
#include "Shaders.h"

using Microsoft::WRL::ComPtr;

class Vertex
{
public:
	Vertex() : Vertex({ 0, 0, 0 }, { 0, 0, 0 }) {}
	Vertex(glm::vec3 position) : position{ position }, color{ 0, 0, 0 } {}
	Vertex(glm::vec3 position, glm::vec3 color) : position{ position }, color{ color } {}
	glm::vec3 position;
	glm::vec3 color;
};

class Renderer
{
public:
	Renderer(HWND hwnd, unsigned width, unsigned height);
	Renderer(const Renderer& other) = delete;
	~Renderer();

	// Initialize what we'll be drawing
	// This will be refactored into a dynamic scene later
	void initScene();

	// Sets the size of the renderer. 
	void resize(unsigned width, unsigned height);

	void render();

private:
	DX11Interface dx11;

	HWND hwnd;
	unsigned width;
	unsigned height;
	bool windowed = true;
	bool vsync = true;

	// Store what we're drawing...this will be refactored
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	int numVertices = 0;
	VertexShaderPtr vertexShader;
	PixelShaderPtr pixelShader;
};