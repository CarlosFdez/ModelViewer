#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Assets.h"
#include "DX11Interface.h"


// Used to load assets for the engine.
// DX11 is hardcoded, replace with a general buffer creator function.
class ResourceManager
{
public:
	void initialize(DX11Interface* dx11);
	MeshResourcePtr loadModel(const std::wstring& relativePath);

private:
	DX11Interface* dx11;
};
