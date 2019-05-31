#include "Renderer.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

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
	dx11 = std::make_unique<DX11Interface>();
	dx11->initialize(hwnd, width, height, windowed);

	resourceManager = std::make_unique<ResourceManager>();
	resourceManager->initialize(dx11.get());

	// Initialize Shaders
	pixelShader = loadPixelShader(dx11->getDevice(), "SimplePixelShader.hlsl");
	vertexShader = loadVertexShader(dx11->getDevice(), "SimpleVertexShader.hlsl");

	// Initialize Constant Buffer
	constantBuffer = dx11->createConstantBuffer<ConstantBufferData>(ConstantBufferData_BLOCKSIZE);
}

Renderer::~Renderer()
{
}

void Renderer::setScene(ScenePtr scene)
{
	this->scene = scene;
}

void Renderer::resize(unsigned width, unsigned height)
{
	dx11->resize(width, height);
}

void Renderer::render()
{
	// Clear background
	dx11->clearView({ 0.0f, 0.0f, 0.0f, 1.0f });

	auto context = dx11->getContext();

	if (scene != nullptr)
	{
		// all of this is the triangle draw call
		for (auto& sceneObject : *scene)
		{
			MeshResourcePtr mesh = sceneObject->mesh;
			if (mesh == nullptr) {
				continue;
			}

			void* rawBuffer = mesh->primitiveBuffers.get();
			D3D11PrimitiveBuffers* buffers = static_cast<D3D11PrimitiveBuffers*>(rawBuffer);
			auto vertexBuffer = buffers->vertexBuffer;
			auto indexBuffer = buffers->indexBuffer;

			// Input assembler stage
			// Set the topology type, vertex information, and the input layout the shaders will use
			context->IASetVertexBuffers(0, 1,
				vertexBuffer->getBufferPtr(),
				vertexBuffer->getStridePtr(),
				vertexBuffer->getOffsetPtr());
			context->IASetIndexBuffer(indexBuffer->get(), indexBuffer->getFormat(), 0);

			// Set shaders and constant buffer. 
			context->IASetInputLayout(vertexShader->inputLayout.Get());
			context->VSSetShader(vertexShader->shader.Get(), nullptr, 0);
			context->PSSetShader(pixelShader->shader.Get(), nullptr, 0);

			// construct MVP matrix
			auto world_m = sceneObject->getModelMatrix();

			glm::vec3 cameraPosition(0, 0, 0);
			glm::vec3 cameraForward(0, 0, 1);
			glm::vec3 cameraUp(0, 1, 0);
			auto view_m = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);

			float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
			auto projection_m = glm::perspective(glm::radians(45.0f), aspectRatio, 0.01f, 50.0f);

			constantBufferData.modelViewProj = projection_m * view_m * world_m;

			// update and assign constant buffer. Buffer goes to register 0.
			constantBuffer->apply(constantBufferData);
			context->VSSetConstantBuffers(0, 1, constantBuffer->getBufferPtr());

			// Render the assets/shaders/triangle.
			context->DrawIndexed(indexBuffer->size(), 0, 0);
		}
	}

	// Finished rendering, present results
	dx11->present(vsync);
}
