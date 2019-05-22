#pragma once

#include <d3d11.h>
#include <direct.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>
#include <memory>

#include "Renderer.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;


ComPtr<ID3DBlob> loadShader(const std::string& relativePath, const char* target, const char* entryPoint = "main");

// Compiles and returns a vertex shader from the shaders folder
VertexShaderDataPtr loadVertexShader(ID3D11Device* device, const std::string& relativePath, const std::string& entryPoint = "main");

// Compiles and returns a pixel (aka fragment) shader from the shaders folder
ComPtr<ID3D11PixelShader> loadFragmentShader(ID3D11Device* device, const std::string& relativePath, const std::string& entryPoint = "main");