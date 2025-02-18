#pragma once 
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Game/GameObject/GameObject.h"

class Scene {
	// TEMP
	friend class Renderer;
public:
	Scene(); 
	~Scene() = default;
public:
	void Update(); 
private:
	std::shared_ptr<TextureManager> mTextureManager{ nullptr };
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{ nullptr };
	std::shared_ptr<MaterialManager> mMaterialManager{ nullptr };

	std::vector<GameObject> mGameObjects{};
};