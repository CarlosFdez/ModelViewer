#include "Renderer.h"

#include <iostream>

#define CHECK_RESULT(result) if (result < 0) { return -1; }

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

// If the pointer to the IUnknown base type is not null, called Release() and set to null.
#define SAFE_RELEASE(x) if (x != nullptr) { x->Release(); x = nullptr; }

Renderer::Renderer(HWND hwnd, unsigned width, unsigned height) : hwnd{ hwnd }, width{ width }, height{ height }
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

	// Init viewport
	viewport.TopLeftX = viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;

	unsigned refreshFrequency = 60; // todo: configure.

	// the swapchain is used to control the presentation of the backbuffer
	// The swap chain must be recreated after each resize
	DXGI_SWAP_CHAIN_DESC swapchainDesc =
	{
		{
			width,
			height,
			{refreshFrequency, 1u},
			DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
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

	// Get the backbuffer of the swap chain
	// The backbuffer is a resource where the render target view points to
	ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf())));

	// TODO: DEPTH BUFFER? STENCIL STATE?

	// Create view discription for the render target. NO CLUE HOW TO CONFIGURE YET OR WHAT IT MEANS
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_RTV renderTargetMipLevelDesc = { 0u };
	renderTargetViewDesc.Texture2D = renderTargetMipLevelDesc;

	// Create the render target. Will be used to render to
	ThrowIfFailed(device->CreateRenderTargetView(
		backbuffer.Get(), // the resource where the render actually goes to (aka backbuffer)
		&renderTargetViewDesc,
		&renderTargetView
	));

	// Bind render target and viewport (todo: add depth stencil?)
	context->OMSetRenderTargets(1, &renderTargetView, nullptr);
	context->RSSetViewports(1, &viewport);
}

Renderer::~Renderer()
{
	SAFE_RELEASE(renderTargetView);
}

void Renderer::setSize(unsigned width, unsigned height)
{
	// todo: determine whether we should resize the swapchain or recreate every time
	this->width = width;
	this->height = height;

	DXGI_MODE_DESC newMode
	{
		this->width,
		this->height,
		{ 60u, 1u }, // refresh rate
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
		DXGI_MODE_SCALING_UNSPECIFIED
	};

	swapchain->ResizeTarget(&newMode);
	swapchain->ResizeBuffers(1, width, height, 
		DXGI_FORMAT_UNKNOWN, 
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

}

#include "loader.h"
#include <glm/vec3.hpp>

std::vector<AdapterData> Renderer::readAdapters()
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

void Renderer::render()
{
	// Clear background
	float color[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, color);

	renderTriangle();

	// Finished rendering, present results
	swapchain->Present((vsync) ? 1 : 0, 0);
}

void Renderer::renderTriangle()
{
	// todo: shader and vertex buffer info needs to be passed in some way...static for now
	static auto pixelShader = loadFragmentShader(device.Get(), "SimplePixelShader.hlsl");
	static auto vertexShader = loadVertexShader(device.Get(), "SimpleVertexShader.hlsl");

	static glm::vec3 triangleVertices[] = {
		glm::vec3(-0.5f, -0.5f, 0),
		glm::vec3(0.0f, 0.5f, 0),
		glm::vec3(0.5f, -0.5f, 0)
	};

	static unsigned short triangleIndices[] = { 0, 1, 2 };

	static D3D11_BUFFER_DESC vertexBufferDesc{
		sizeof(glm::vec3) * ARRAYSIZE(triangleVertices),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, 0, 0  // flags and stride all 0
	};

	static D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = triangleVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	static ComPtr<ID3D11Buffer> vertexBuffer;
	if (vertexBuffer.Get() == nullptr)
	{
		ThrowIfFailed(device->CreateBuffer(
			&vertexBufferDesc,
			&vertexBufferData,
			vertexBuffer.GetAddressOf()
		));
	}

	static D3D11_BUFFER_DESC indexBufferDesc = {
		sizeof(unsigned short) * ARRAYSIZE(triangleIndices),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0, 0, 0 // flags and stride all 0
	};

	static D3D11_SUBRESOURCE_DATA indexBufferData = {
		triangleIndices,
		0,
		0
	};

	static ComPtr<ID3D11Buffer> indexBuffer;
	if (indexBuffer.Get() == nullptr)
	{
		ThrowIfFailed(device->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			indexBuffer.GetAddressOf()
		));
	}

	// all of this is the triangle draw call
	{
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		unsigned stride = sizeof(glm::vec3);
		unsigned offset = 0;

		// Input assembler stage
		// Set the topology type, vertex information, and the input layout the shaders will use
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		context->IASetInputLayout(vertexShader->inputLayout.Get());

		// Set shaders
		context->VSSetShader(vertexShader->shader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader.Get(), NULL, 0);

		// Render the assets/shaders/triangle.
		context->DrawIndexed(ARRAYSIZE(triangleIndices), 0, 0);
	}
}