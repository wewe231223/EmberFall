#pragma once 
#include <fstream>
#include "../Renderer/Manager/RenderManager.h"

class MaterialFileLoader
{
public:
    MaterialFileLoader() = default;
    MaterialFileLoader(std::shared_ptr<RenderManager> renderManager);

	~MaterialFileLoader() = default;

	MaterialFileLoader(const MaterialFileLoader&) = default;
	MaterialFileLoader& operator=(const MaterialFileLoader&) = default;

	MaterialFileLoader(MaterialFileLoader&&) = default;
	MaterialFileLoader& operator=(MaterialFileLoader&&) = default;
public:
    MaterialConstants Load(const std::filesystem::path& materialFilePath); 

private:
	std::shared_ptr<RenderManager> mRenderManager;
};
