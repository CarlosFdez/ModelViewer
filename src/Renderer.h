#pragma once

#include <vector>
#include <memory>
#include <wrl/client.h>

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "Assets.h"
#include "Scene.h"
#include "DX11Interface.h"
#include "ResourceManager.h"

using Microsoft::WRL::ComPtr;

// Data for a constant buffer
struct ConstantBufferData
{
	// final matrix multiplication applied to a shader;
	glm::mat4x4 modelViewProj = glm::mat4x4(1.0f); // [64 bytes] [4 blocks]
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
	void setScene(ScenePtr scene);

	// Sets the size of the renderer. 
	void resize(unsigned width, unsigned height);

	ResourceManager* getResourceManager() { return resourceManager.get();  }

	void render();

private:
	std::unique_ptr<DX11Interface> dx11;
	std::unique_ptr<ResourceManager> resourceManager;

	VertexShaderPtr vertexShader = nullptr;
	PixelShaderPtr pixelShader = nullptr;

	HWND hwnd;
	unsigned width;
	unsigned height;
	bool windowed = true;
	bool vsync = true;

	// Stores what we're drawing
	ScenePtr scene;

	ConstantBufferPtr<ConstantBufferData> constantBuffer;
	ConstantBufferData constantBufferData;
};