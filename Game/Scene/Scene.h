#pragma once 
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/GameObject/GameObject.h"
#include "../Game/Scene/Camera.h"
#include "../Game/GameObject/Animator.h"
#include "../MeshLoader/Loader/TerrainLoader.h"


class Scene {
public:
	Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList ,std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation); 
	~Scene() = default;
public:
	void Update();
public:
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList); 
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);
	void PaintTree(size_t treeCount); 
private:
	std::shared_ptr<TextureManager> mTextureManager{ nullptr };
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{ nullptr };
	std::shared_ptr<MaterialManager> mMaterialManager{ nullptr };

	std::unordered_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	std::unordered_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};

	Camera mCamera{};
	std::unique_ptr<CameraMode> mCameraMode{ nullptr };

	std::vector<GameObject> mGameObjects{};

	GameObject mSkyBox{}; 

	TerrainLoader tLoader{}; 

	TextBlock* mPickedObjectText{ TextBlockManager::GetInstance().CreateTextBlock(L"",D2D1_RECT_F{100.f,0.f,800.f,100.f},StringColor::Black, "NotoSansKR") };
};