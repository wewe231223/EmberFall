#pragma once 
#include <array>
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Renderer/Manager/ParticleManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../Game/GameObject/GameObject.h"
#include "../Game/Scene/Camera.h"
#include "../Game/GameObject/Animator.h"
#include "../MeshLoader/Loader/TerrainLoader.h"
#include "../Game/Scene/Player.h"
#include "../ServerLib/PacketProcessor.h"
#include "../ServerLib/PacketHandler.h"
class Scene {
public:
	Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList ,std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation); 
	~Scene();
public:
	void ProcessNetwork(); 
	void Update();
	void SendNetwork(); 

private:
	void BuildPacketProcessor(); 
	void BuildSendKeyList(); 

	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList); 
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildAniamtionController(); 

	void BuildBaseAnimationController();
	void BuildArcherAnimationController();
	void BuildSwordManAnimationController();
	void BuildMageAnimationController();

	void BuildMonsterType1AnimationController();

	void SetInputBaseAnimMode(); 
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
	void ProcessPacketAnimation(PacketHeader* header);
private:
	std::shared_ptr<TextureManager> mTextureManager{ nullptr };
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{ nullptr };
	std::shared_ptr<MaterialManager> mMaterialManager{ nullptr };
	std::shared_ptr<ParticleManager> mParticleManager{ nullptr };

	std::unordered_map<std::string, Collider> mColliderMap{};
	std::unordered_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	std::unordered_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};

	ClientPacketProcessor mPacketProcessor{};

	Camera mCamera{};
	std::unique_ptr<CameraMode> mCameraMode{ nullptr };


	std::unordered_map<NetworkObjectIdType, GameObject*> mGameObjectMap{};
	std::vector<GameObject> mGameObjects{};

	int mNetworkSign{};
	int mInputSign{}; 
	std::vector<DirectX::Keyboard::Keys> mSendKeyList{};

	std::unordered_map<NetworkObjectIdType, Player*> mPlayerIndexmap{};
	std::array<Player, 5> mPlayers{ Player{}, };
	
	Player* mMyPlayer{ nullptr };

	

	std::array<GameObject, 3> mWeapons{}; 
	
	AnimatorGraph::BoneMaskAnimationGraphController mBaseAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mArcherAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mSwordManAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mMageAnimationController{};

	AnimatorGraph::AnimationGraphController mMonsterType1AnimationController{}; 

	GameObject mSkyBox{};

	TerrainLoader tLoader{}; 
	TerrainCollider tCollider{};

	Particle test{};
	Particle test1{};
	Particle test2{}; 

	TextBlock* mNetworkInfoText{ TextBlockManager::GetInstance().CreateTextBlock(L"",D2D1_RECT_F{100.f,0.f,800.f,100.f},StringColor::Black, "NotoSansKR") };
};