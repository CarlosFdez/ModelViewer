#pragma once

#include <d3d11.h>
#include <direct.h>
#include <wrl/client.h>

#include <array>
#include <vector>
#include <memory>

#include "Shaders.h"
#include "Assets.h"

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


// Class used to encapsulate a vertex buffer, which is used to
// assign vertices to the pipeline for a single draw call.
// Note that to connect them into triangles, an IndexBuffer is required.
// Must be inserted into the pipeline before the draw call.
class VertexBuffer
{
public:
	VertexBuffer(ID3D11Buffer* buffer, unsigned numVertices, unsigned stride):
		buffer{ buffer }, numVertices{ numVertices }, stride{ stride } {}

	ID3D11Buffer* const* getBufferPtr() const { return buffer.GetAddressOf(); }
	const unsigned* getStridePtr() const { return &stride; }
	const unsigned* getOffsetPtr() const { return &offset; }

private:
	ComPtr<ID3D11Buffer> buffer;
	const unsigned numVertices;
	const unsigned stride;
	const unsigned offset = 0;
};


// Encapsulates an index buffer, which is used to connect vertices into triangles.
// Must be inserted into the pipeline before the draw call.
class IndexBuffer
{
public:
	IndexBuffer(ID3D11Buffer* buffer, unsigned numIndices) :
		buffer{ buffer }, numIndices{ numIndices } {}

	// Returns a pointer to the managed low level buffer
	ID3D11Buffer* get() const { return buffer.Get(); }

	// Returns the number of indices
	unsigned size() const { return numIndices; }

	// Returns the index format. Current hardcoded to 32bit, however mobile devices only support 16bit.
	DXGI_FORMAT getFormat() const { return DXGI_FORMAT_R32_UINT; }

private:
	ComPtr<ID3D11Buffer> buffer;
	const unsigned numIndices;
};


// Class used to encapsulate a constant buffer.
// Constant buffers are a way of passing global data (like the model view projection matrix)
// to a shader.
template <typename T>
class ConstantBuffer
{
public:
	ConstantBuffer(ComPtr<ID3D11DeviceContext> context, ID3D11Buffer* buffer, unsigned blockSize):
		context{ context }, buffer{ buffer }, blockSize{ blockSize } {}

	ID3D11Buffer* const* getBufferPtr() const { return buffer.GetAddressOf(); }
	unsigned sizeOf() const { return blockSize;  }

	// Applies 
	void apply(const T& data)
	{
		// update constant buffer. Note that NVidia recommends map/discard over UpdateSubResource()
		D3D11_MAPPED_SUBRESOURCE cbResource;
		context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cbResource);
		CopyMemory(cbResource.pData, &data, sizeOf());
		context->Unmap(buffer.Get(), 0);
	}


private:
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<ID3D11Buffer> buffer;
	const unsigned blockSize;
};

typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;
typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;

template <typename T>
using ConstantBufferPtr = std::shared_ptr<ConstantBuffer<T>>;


// Buffers associated with a particular mesh
struct D3D11PrimitiveBuffers
{
	VertexBufferPtr vertexBuffer;
	IndexBufferPtr indexBuffer;
};


// Main class used to interface with the DirectX 11 runtime.
// Contains the driver and context classes used to perform rendering,
// as well as helper methods to load elements like vertex buffers.
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

	template <typename T>
	ConstantBufferPtr<T> createConstantBuffer(unsigned blockSize)
	{
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constantBufferDesc.ByteWidth = blockSize;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // modifiable
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // cpu modifiable

		ID3D11Buffer* buffer;
		ThrowIfFailed(device->CreateBuffer(
			&constantBufferDesc,
			0,
			&buffer
		));

		return std::make_shared<ConstantBuffer<T>>(context, buffer, blockSize);
	}

	VertexBufferPtr createVertexBuffer(const void* verticesPtr, unsigned numVertices, size_t stride);
	IndexBufferPtr createIndexBuffer(const unsigned* indicesPtr, unsigned numIndices);

	template <typename T>
	inline VertexBufferPtr createVertexBuffer(const std::vector<T>& vertices)
	{
		return createVertexBuffer(vertices.data(), vertices.size(), sizeof(T));
	}

	inline IndexBufferPtr createIndexBuffer(const std::vector<unsigned> indices)
	{
		return createIndexBuffer(indices.data(), indices.size());
	}

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