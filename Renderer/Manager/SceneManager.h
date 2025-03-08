#pragma once 
#include <memory>
#include "../Utility/DirectXInclude.h"
#include "../Manager/MeshRenderManager.h"
#include "../Manager/TextureManager.h"

class SceneManager {
	friend class Renderer;
public:
	SceneManager() = default;
	~SceneManager() = default;

	SceneManager(const SceneManager& other) = default;
	SceneManager& operator=(const SceneManager& other) = default;

	SceneManager(SceneManager&& other) = default;
	SceneManager& operator=(SceneManager&& other) = default;
private:
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{ nullptr };
	std::shared_ptr<TextureManager> mTextureManager{ nullptr };
	std::shared_ptr<MaterialManager> mMaterialManager{ nullptr };

	
};