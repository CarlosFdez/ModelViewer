#pragma once

#include <d3d11.h>
#include <direct.h>
#include <vector>
#include <memory>
#include <wrl/client.h>

#include <glm/vec3.hpp>

#include "Shaders.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

struct AdapterData
{
	ComPtr<IDXGIAdapter> adapter;
	DXGI_ADAPTER_DESC description;
};


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
	// Reads all adapters. Corresponds to display out devices
	void updateRenderTarget();
	void updateViewport();
	std::vector<AdapterData> readAdapters();

	HWND hwnd;
	unsigned width;
	unsigned height;
	bool windowed = true;
	bool vsync = true;

	// This is the DirectX Graphics Infrastructure. Used to:
	// - Check what devices we have and what they can do
	ComPtr<IDXGIFactory> factory;
	AdapterData adapter;

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapchain;
	ComPtr<ID3D11Texture2D> backbuffer;
	ComPtr<ID3D11RenderTargetView> renderTargetView;

	// Rasterizer state (used to cull and transform vertices before render)
	ComPtr<ID3D11RasterizerState> rasterizerState;

	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	// Sampler state, used to sample textures in shaders
	ComPtr<ID3D11SamplerState> samplerState;

	D3D_FEATURE_LEVEL featureLevel;
	D3D11_VIEWPORT viewport;

	// Store what we're drawing...this will be refactored
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	int numVertices = 0;
	VertexShaderPtr vertexShader;
	PixelShaderPtr pixelShader;
};