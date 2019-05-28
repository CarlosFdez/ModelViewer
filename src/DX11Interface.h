#pragma once

#include <d3d11.h>
#include <direct.h>
#include <wrl/client.h>

#include <array>
#include <vector>

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

class DX11Interface
{
public:
	ID3D11Device* getDevice() const { return device.Get(); }
	ID3D11DeviceContext* getContext() const { return context.Get(); }

	void initialize(HWND hwnd, unsigned width, unsigned height, bool windowed);
	void resize(unsigned width, unsigned height);

	// Reads all adapters. Corresponds to display out devices
	std::vector<AdapterData> readAdapters();

	// Clears the view to a specific color. Call before each render
	void clearView(std::array<float, 4> color);

	// Presents the rendered view for display
	void present(bool vsync);

private:
	void updateRenderTarget(unsigned width, unsigned height);
	void updateViewport(unsigned width, unsigned height);

	ComPtr<IDXGIFactory> factory;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapchain;
	ComPtr<ID3D11Texture2D> backbuffer;
	ComPtr<ID3D11RenderTargetView> renderTargetView;

	AdapterData adapter;

	// Rasterizer state (used to cull and transform vertices before render)
	ComPtr<ID3D11RasterizerState> rasterizerState;

	D3D_FEATURE_LEVEL featureLevel;
	D3D11_VIEWPORT viewport;

	// Depth stencil, used to process depth
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	// Sampler state, used to sample textures in shaders
	ComPtr<ID3D11SamplerState> samplerState;
};