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
	unsigned short triangleIndices[] = { 0, 1, 2 };

	vertexBuffer = dx11.createVertexBuffer(triangleVertices, ARRAYSIZE(triangleVertices), sizeof(Vertex));
	indexBuffer = dx11.createIndexBuffer(triangleIndices, ARRAYSIZE(triangleIndices), sizeof(unsigned short));
	constantBuffer = dx11.createConstantBuffer<ConstantBufferData>(ConstantBufferData_BLOCKSIZE);
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
		// Input assembler stage
		// Set the topology type, vertex information, and the input layout the shaders will use
		context->IASetVertexBuffers(0, 1, 
			vertexBuffer->getBufferPtr(), 
			vertexBuffer->getStridePtr(), 
			vertexBuffer->getOffsetPtr());
		context->IASetIndexBuffer(indexBuffer->get(), DXGI_FORMAT_R16_UINT, 0);

		// update constant buffer
		constantBufferData.modelViewProj = glm::mat4x4(1.0f);
		constantBuffer->apply(constantBufferData);

		// Set shaders and constant buffer. Buffer goes to register 0.
		context->IASetInputLayout(vertexShader->inputLayout.Get());
		context->VSSetConstantBuffers(0, 1, constantBuffer->getBufferPtr());
		context->VSSetShader(vertexShader->shader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader->shader.Get(), nullptr, 0);

		// Render the assets/shaders/triangle.
		context->DrawIndexed(indexBuffer->size(), 0, 0);
	}

	// Finished rendering, present results
	dx11.present(vsync);
}
