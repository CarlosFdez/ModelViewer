#include "Renderer.h"

#include <iostream>

#define CHECK_RESULT(result) { if (result < 0) { return -1; } }

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

Renderer::Renderer(HWND hwnd, unsigned width, unsigned height) : hwnd{ hwnd }, width{ width }, height{ height }
{
	dx11.initialize(hwnd, width, height, windowed);

	// initializes the scene - todo: refactor this out for a dynamic set
	initScene();
}

Renderer::~Renderer()
{
}


void Renderer::initScene()
{
	auto device = dx11.getDevice();
	auto context = dx11.getContext();

	// todo: shader and vertex buffer info needs to be passed in some way...static for now
	pixelShader = loadPixelShader(device, "SimplePixelShader.hlsl");
	vertexShader = loadVertexShader(device, "SimpleVertexShader.hlsl");

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
	dx11.resize(width, height);
}

void Renderer::render()
{
	// Clear background
	dx11.clearView({ 0.0f, 0.0f, 0.0f, 1.0f });

	auto context = dx11.getContext();

	// all of this is the triangle draw call
	{
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		unsigned stride = sizeof(Vertex);
		unsigned offset = 0;

		// Input assembler stage
		// Set the topology type, vertex information, and the input layout the shaders will use
		context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		context->IASetInputLayout(vertexShader->inputLayout.Get());

		// Set shaders
		context->VSSetShader(vertexShader->shader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader->shader.Get(), nullptr, 0);

		// Render the assets/shaders/triangle.
		context->DrawIndexed(numVertices, 0, 0);
	}

	// Finished rendering, present results
	dx11.present(vsync);
}
