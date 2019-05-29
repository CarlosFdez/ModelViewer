#include "DX11Interface.h"

#include <iostream>


#include <comdef.h>

#define CHECK_RESULT(result) { if (result < 0) { return -1; } }

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		_com_error error(hr);
		auto message = error.ErrorMessage();
		throw std::exception(message);
	}
}

void DX11Interface::initialize(HWND hwnd, unsigned width, unsigned height, bool windowed)
{
	// Init DXGI factory
	ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(factory.GetAddressOf())));

	// Read adapters. For now we pick the default one
	auto adapters = readAdapters();
	this->adapter = adapters[0];
	std::cout << "There are " << adapters.size() << " adapters. Picking the first one" << std::endl;

	D3D_FEATURE_LEVEL featureLevelInputs[7] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create Device to use for future operations
	ThrowIfFailed(D3D11CreateDevice(
		this->adapter.adapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN, // must be unknown if adapter is nonnull
		NULL,
		D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG,
		featureLevelInputs,
		ARRAYSIZE(featureLevelInputs),
		D3D11_SDK_VERSION,

		// results
		this->device.GetAddressOf(),
		&this->featureLevel,
		&this->context
	));

	unsigned refreshFrequency = 60; // todo: configure.

	// the swapchain is used to control the presentation of the backbuffer
	// The swap chain must be recreated after each resize
	DXGI_SWAP_CHAIN_DESC swapchainDesc =
	{
		{
			width,
			height,
			{refreshFrequency, 1u},
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
			DXGI_MODE_SCALING_UNSPECIFIED
		}, // BUFFER DESC
		{1u, 0u}, // n-samples and quality
		DXGI_CPU_ACCESS_NONE | DXGI_USAGE_RENDER_TARGET_OUTPUT,
		1u,
		hwnd,
		windowed,
		DXGI_SWAP_EFFECT_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};
	ThrowIfFailed(factory->CreateSwapChain(device.Get(), &swapchainDesc, swapchain.GetAddressOf()));

	// Get the backbuffer of the swap chain (what we render to)
	ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf())));

	// create the render target and viewport
	updateRenderTarget(width, height);
	updateViewport(width, height);

	// Create the rasterizer state. This performs vertex transformations and culling
	{
		D3D11_RASTERIZER_DESC rasterizerDesc
		{
			D3D11_FILL_MODE::D3D11_FILL_SOLID,
			D3D11_CULL_MODE::D3D11_CULL_BACK,
			false, // draw clockwise
			0,
			0,
			0,
			0,
			0,
			0,
			0,
		};

		ThrowIfFailed(device->CreateRasterizerState(
			&rasterizerDesc,
			rasterizerState.GetAddressOf()
		));
	}

	// Create samplerstate (used to process textures)
	{
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; // texture scaling?
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ThrowIfFailed(device->CreateSamplerState(
			&samplerDesc,
			samplerState.GetAddressOf()
		));
	}
}

void DX11Interface::resize(unsigned width, unsigned height)
{
	// note: resize target is disabled as the window is already resized by CrossWindow
	// ResizeTarget also updates the window size. If we wanna use it, we need CrossWindow to not do resizing
	//DXGI_MODE_DESC newMode
	//{
	//	this->width,
	//	this->height,
	//	{ 60u, 1u }, // refresh rate
	//	DXGI_FORMAT_R8G8B8A8_UNORM,
	//	DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
	//	DXGI_MODE_SCALING_UNSPECIFIED
	//};

	//// resize the buffer. Using 0 for width and height makes the sizing automatic
	//swapchain->ResizeTarget(&newMode);

	// Release backbuffer and render target smart pointers
	backbuffer = nullptr;
	renderTargetView = nullptr;

	swapchain->ResizeBuffers(1, 0, 0,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// Get the backbuffer of the swap chain (what we render to)
	ThrowIfFailed(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.GetAddressOf()));

	// recreate the render target
	updateRenderTarget(width, height);
	updateViewport(width, height);
}

void DX11Interface::updateRenderTarget(unsigned width, unsigned height)
{
	// Create the Depth Stencil Buffer and view, 
	// which is a "depth" buffer" used to control which pixels are visible at each point
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;

	ThrowIfFailed(device->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthStencilBuffer.GetAddressOf()));
	ThrowIfFailed(device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf()));

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilStateDesc.DepthEnable = true;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	ThrowIfFailed(device->CreateDepthStencilState(&depthStencilStateDesc, depthStencilState.GetAddressOf()));

	// Create view discription for the render target. NO CLUE HOW TO CONFIGURE YET OR WHAT IT MEANS
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_RTV renderTargetMipLevelDesc = { 0u };
	renderTargetViewDesc.Texture2D = renderTargetMipLevelDesc;

	// Create the render target. Will be used to render to
	ThrowIfFailed(device->CreateRenderTargetView(
		backbuffer.Get(), // the resource where the render actually goes to (aka backbuffer)
		&renderTargetViewDesc,
		renderTargetView.GetAddressOf()
	));

	// Bind render target and viewport (todo: add depth stencil?)
	context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
}

void DX11Interface::updateViewport(unsigned width, unsigned height)
{
	// Init viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;

	context->RSSetViewports(1, &viewport);
}

std::vector<AdapterData> DX11Interface::readAdapters()
{
	std::vector<AdapterData> adapters;

	int i = 0;
	ComPtr<IDXGIAdapter> adapter;
	while (factory->EnumAdapters(i++, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		AdapterData result;
		result.adapter = adapter;
		adapter->GetDesc(&result.description);

		adapters.push_back(result);
	}

	return adapters;
}

void DX11Interface::clearView(std::array<float, 4> color)
{
	context->ClearRenderTargetView(renderTargetView.Get(), &color[0]);
	context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, D3D11_MAX_DEPTH, 0);

	// todo: figure out if this has to be done on every command, or once per draw phase
	{
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set Rasterizer State (used to cull and transform vertices before the shaders)
		context->RSSetState(rasterizerState.Get());
		context->OMSetDepthStencilState(depthStencilState.Get(), 0);
	}
}

void DX11Interface::present(bool vsync)
{
	swapchain->Present((vsync) ? 1 : 0, 0);
}

VertexBufferPtr DX11Interface::createVertexBuffer(void* verticesPtr, unsigned numVertices, size_t stride)
{
	unsigned strideU = static_cast<unsigned>(stride);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.ByteWidth = strideU * numVertices;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = verticesPtr;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	ID3D11Buffer* buffer;
	ThrowIfFailed(device->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&buffer
	));

	return std::make_shared<VertexBuffer>(buffer, numVertices, strideU);
}

IndexBufferPtr DX11Interface::createIndexBuffer(void* indicesPtr, unsigned numIndices, size_t stride)
{
	static D3D11_BUFFER_DESC indexBufferDesc = {
		stride * numIndices,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0, 0, 0 // flags and stride all 0
		};

	static D3D11_SUBRESOURCE_DATA indexBufferData = {
		indicesPtr,
		0,
		0
	};

	ID3D11Buffer* buffer;
	ThrowIfFailed(device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&buffer
	));

	return std::make_shared<IndexBuffer>(buffer, numIndices);
}
