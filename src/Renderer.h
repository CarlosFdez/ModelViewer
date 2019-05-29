#pragma once

#include <vector>
#include <memory>
#include <wrl/client.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

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

// Data for a constant buffer
struct ConstantBufferData
{
	// final matrix multiplication applied to a shader;
	glm::mat4x4 modelViewProj; // [64 bytes] [4 blocks]
};

const size_t ConstantBufferData_BLOCKSIZE = 64;

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
	VertexBufferPtr vertexBuffer;
	IndexBufferPtr indexBuffer;
	VertexShaderPtr vertexShader;
	PixelShaderPtr pixelShader;

	ConstantBufferPtr<ConstantBufferData> constantBuffer;
	ConstantBufferData constantBufferData;
};