#pragma once

#include <d3d11.h>
#include <direct.h>
#include <vector>
#include <memory>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

struct VertexShader
{
	ComPtr<ID3D11VertexShader> shader;
	ComPtr<ID3DBlob> shaderBuffer;
	ComPtr<ID3D11InputLayout> inputLayout;
};

struct AdapterData
{
	ComPtr<IDXGIAdapter> adapter;
	DXGI_ADAPTER_DESC description;
};

typedef std::shared_ptr<VertexShader> VertexShaderDataPtr;

class Renderer
{
public:
	Renderer(HWND hwnd, unsigned width, unsigned height);
	Renderer(const Renderer& other) = delete;
	~Renderer();

	// Sets the size of the renderer. 
	void setSize(unsigned width, unsigned height);

	void render();

	// example render functions. Will be replaced in the future
	void renderTriangle();

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

	D3D_FEATURE_LEVEL featureLevel;
	D3D11_VIEWPORT viewport;

	ComPtr<ID3D11RenderTargetView> renderTargetView;
};