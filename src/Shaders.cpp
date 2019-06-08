#include "Shaders.h"

#include <iostream>
#include <filesystem>

using namespace std;
using Microsoft::WRL::ComPtr;

ComPtr<ID3DBlob> loadShader(const std::string& relativePath, const char* target, const char* entryPoint)
{
	auto path = filesystem::current_path();
	path.append("assets\\shaders");
	path.append(relativePath);

#ifdef _DEBUG
	unsigned compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	unsigned compileFlags = 0;
#endif

	if (!filesystem::exists(path))
	{
		std::string msg("Could not find shader file " + path.string());
		std::cout << msg << endl;
		throw std::exception(msg.c_str());
	}

	cout << "Compiling Shader " << relativePath << endl;

	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3DCompileFromFile(
		path.wstring().c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint,
		target,
		compileFlags,
		0,
		compiledShader.GetAddressOf(),
		errors.GetAddressOf());

	if (FAILED(result))
	{
		const char* errorString = static_cast<const char*>(errors->GetBufferPointer());
		std::cerr << "ERROR: " << relativePath << ": " << errorString << std::endl;
		throw std::exception(errorString);
	}

	return compiledShader;
}

VertexShaderPtr loadVertexShader(ID3D11Device* device, const std::string& relativePath, const std::string& entryPoint)
{
	VertexShaderPtr shader(new VertexShader());
	shader->shaderBuffer = loadShader(relativePath, "vs_5_0", entryPoint.c_str());

	HRESULT result;
	// todo: do something with result

	result = device->CreateVertexShader(
		shader->shaderBuffer->GetBufferPointer(),
		shader->shaderBuffer->GetBufferSize(),
		nullptr,
		shader->shader.GetAddressOf()
	);

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	// note: in the current shader file, its a vec2
	static D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
		// each line marks a different variable in the input and output struct
		// todo: if the shader becomes 2D...should be an R32G32B32_FLOAT
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	result = device->CreateInputLayout(
		vertexLayoutDesc,
		ARRAYSIZE(vertexLayoutDesc),
		shader->shaderBuffer->GetBufferPointer(),
		shader->shaderBuffer->GetBufferSize(),
		shader->inputLayout.GetAddressOf()
	);

	return shader;
}

PixelShaderPtr loadPixelShader(ID3D11Device* device, const std::string& relativePath, const std::string& entryPoint)
{
	PixelShaderPtr shader(new PixelShader());
	shader->shaderBuffer = loadShader(relativePath, "ps_5_0", entryPoint.c_str());

	HRESULT result = device->CreatePixelShader(
		shader->shaderBuffer->GetBufferPointer(),
		shader->shaderBuffer->GetBufferSize(),
		nullptr,
		shader->shader.GetAddressOf()
	);

	return shader;
}