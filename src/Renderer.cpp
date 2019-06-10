#include "Renderer.h"

#include <iostream>

#include <glm/glm.hpp>

#define CHECK_RESULT(result) { if (result < 0) { return -1; } }

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

Renderer::Renderer(xwin::Window& window)
{
	this->hwnd = window.getDelegate().hwnd;
	this->width = window.getDesc().width;
	this->height = window.getDesc().height;

	dx11 = std::make_unique<DX11Interface>();
	dx11->initialize(hwnd, width, height, windowed);

	resourceManager = std::make_unique<ResourceManager>();
	resourceManager->initialize(dx11.get());

	inputManager = std::make_unique<InputManager>();

	// Initialize Shaders
	pixelShader = loadPixelShader(dx11->getDevice(), "SimplePixelShader.hlsl");
	vertexShader = loadVertexShader(dx11->getDevice(), "SimpleVertexShader.hlsl");

	// Initialize Constant Buffer
	constantBuffer = dx11->createConstantBuffer<ConstantBufferData>(ConstantBufferData_BLOCKSIZE);

	// Initialize camera
	camera = std::make_unique<Camera>();
	camera->setFov(45.0f);
	camera->setAspectRatio(width, height);
	camera->setClipRange(0.1f, 50.0f);
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
	if (width == 0 || height == 0)
	{
		std::cout << "Ignoring invalid resize request [" << width << ", " << height << "]" << std::endl;
		return;
	}

	this->width = width;
	this->height = height;

	dx11->resize(width, height);
	camera->setAspectRatio(width, height);
}

void Renderer::handleEvent(const xwin::Event& event)
{
	if (event.type == xwin::EventType::Resize)
	{
		// resize renderer
		xwin::ResizeData data = event.data.resize;
		this->resize(data.width, data.height);
	}
	else if (event.type == xwin::EventType::Focus)
	{
		xwin::FocusData data = event.data.focus;
		if (!data.focused)
		{
			std::cout << "Window unfocused" << std::endl;
			inputManager->notifyLostFocus();
		}
	}
	else if (event.type == xwin::EventType::Keyboard)
	{
		xwin::KeyboardData data = event.data.keyboard;
		inputManager->notifyKeyStateChange(data.key, data.state);
	}
	else if (event.type == xwin::EventType::MouseInput)
	{
		xwin::MouseInputData data = event.data.mouseInput;
		inputManager->notifyMouseButtonChange(data.button, data.state);
	}
	else if (event.type == xwin::EventType::MouseRaw)
	{
		xwin::MouseRawData data = event.data.mouseRaw;
		inputManager->notifyMouseRawInput(data.deltax, data.deltay);
	}
}

void Renderer::render()
{
	// Clear background
	dx11->clearView({ 0.0f, 0.0f, 0.0f, 1.0f });

	auto context = dx11->getContext();
	auto viewProjectionMatrix = camera->getViewProjectionMatrix();

	// Render the scene
	if (scene != nullptr)
	{
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

			// Set shaders
			context->IASetInputLayout(vertexShader->inputLayout.Get());
			context->VSSetShader(vertexShader->shader.Get(), nullptr, 0);
			context->PSSetShader(pixelShader->shader.Get(), nullptr, 0);

			// Assign matrices to constant buffer
			constantBufferData.model = sceneObject->getModelMatrix();
			constantBufferData.viewProjection = viewProjectionMatrix;

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
