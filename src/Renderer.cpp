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
	updateRenderTarget();
	updateViewport();

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

	// initializes the scene - todo: refactor this out for a dynamic set
	initScene();
}

Renderer::~Renderer()
{
}

void Renderer::initScene()
{
	// todo: shader and vertex buffer info needs to be passed in some way...static for now
	pixelShader = loadPixelShader(device.Get(), "SimplePixelShader.hlsl");
	vertexShader = loadVertexShader(device.Get(), "SimpleVertexShader.hlsl");

	// note: vertices must be clockwise (or it'll fail)
	Vertex triangleVertices[] = {
		Vertex({-0.5f, -0.5f, 0}, { 1, 0, 0}),
		Vertex({0.0f, 0.5f, 0}, { 0, 1, 0}),
		Vertex({0.5f, -0.5f, 0}, { 0, 1, 1})
	};
	this->numVertices = ARRAYSIZE(triangleVertices);

	unsigned short triangleIndices[] = { 0, 1, 2 };

	D3D11_BUFFER_DESC vertexBufferDesc{
		sizeof(Vertex) * this->numVertices,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, 0, 0  // flags and stride all 0
	};

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = triangleVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	ThrowIfFailed(device->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		vertexBuffer.GetAddressOf()
	));

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

	ThrowIfFailed(device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		indexBuffer.GetAddressOf()
	));
}

void Renderer::resize(unsigned width, unsigned height)
{
	// todo: determine whether we should resize the swapchain or recreate every time
	this->width = width;
	this->height = height;

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
	ThrowIfFailed(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) backbuffer.GetAddressOf()));

	// recreate the render target
	updateRenderTarget();
	updateViewport();
}

void Renderer::updateRenderTarget()
{
	// Create the Depth Stencil Buffer and view, 
	// which is a "depth" buffer" used to control which pixels are visible at each point
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	depthStencilBufferDesc.Width = this->width;
	depthStencilBufferDesc.Height = this->height;
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

void Renderer::updateViewport()
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
	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(renderTargetView.Get(), color);
	context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, D3D11_MAX_DEPTH, 0);

	// all of this is the triangle draw call
	{
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		unsigned stride = sizeof(Vertex);
		unsigned offset = 0;

		// Input assembler stage
		// Set the topology type, vertex information, and the input layout the shaders will use
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		context->IASetInputLayout(vertexShader->inputLayout.Get());

		// Set Rasterizer State (used to cull and transform vertices before the shaders)
		context->RSSetState(rasterizerState.Get());
		context->OMSetDepthStencilState(depthStencilState.Get(), 0);

		// Set shaders
		context->VSSetShader(vertexShader->shader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader->shader.Get(), nullptr, 0);

		// Render the assets/shaders/triangle.
		context->DrawIndexed(numVertices, 0, 0);
	}

	// Finished rendering, present results
	swapchain->Present((vsync) ? 1 : 0, 0);
}
