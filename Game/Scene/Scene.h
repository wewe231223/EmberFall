#pragma once 
#include <array>
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/GameObject/GameObject.h"
#include "../Game/Scene/Camera.h"
#include "../Game/GameObject/Animator.h"
#include "../MeshLoader/Loader/TerrainLoader.h"
#include "../Game/Scene/Player.h"
#include "../ServerLib/PacketProcessor.h"
#include "../ServerLib/PacketHandler.h"

class Scene {
public:
	Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList ,std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation); 
	~Scene() = default;
public:
	void ProcessNetwork(); 
	void Update();
public:
	void BuildPacketProcessor(); 

	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList); 
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildAniamtionController(); 

	void SetInputArcherMode(); 
	void SetInputSwordManMode();
	void SetInputMageMode();
private:
	void ProcessNotifyId(PacketHeader* header);
	void ProcessPacketProtocolVersion(PacketHeader* header);
	void ProcessPlayerPacket(PacketHeader* header);
	void ProcessObjectPacket(PacketHeader* header);
	void ProcessObjectDead(PacketHeader* header);
	void ProcessObjectAppeared(PacketHeader* header);
	void ProcessObjectDisappeared(PacketHeader* header);
	void ProcessPlayerExit(PacketHeader* header);
	void ProcessAcquiredItem(PacketHeader* header);
	void ProcessObjectAttacked(PacketHeader* header);
	void ProcessUseItem(PacketHeader* header);
	void ProcessRestoreHP(PacketHeader* header);
private:
	std::shared_ptr<TextureManager> mTextureManager{ nullptr };
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{ nullptr };
	std::shared_ptr<MaterialManager> mMaterialManager{ nullptr };

	std::unordered_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	std::unordered_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};

	ClientPacketProcessor mPacketProcessor{};

	Camera mCamera{};
	std::unique_ptr<CameraMode> mCameraMode{ nullptr };

	std::vector<GameObject> mGameObjects{};

	int mNetworkSign{};
	int mInputSign{}; 
	std::array<Player, 4> mHumanPlayers{};
	std::array<Player, 4>::iterator mMyPlayer{}; 
	
	AnimatorGraph::BoneMaskAnimationGraphController mArcherAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mSwordManAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mMageAnimationController{};

	GameObject mSkyBox{}; 

	TerrainLoader tLoader{}; 
	TerrainCollider tCollider{};

	TextBlock* mNetworkInfoText{ TextBlockManager::GetInstance().CreateTextBlock(L"",D2D1_RECT_F{100.f,0.f,800.f,100.f},StringColor::Black, "NotoSansKR") };
};