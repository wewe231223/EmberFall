#pragma once 
#include <array>
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Renderer/Manager/ParticleManager.h"
#include "../Renderer/Render/Canvas.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../Game/GameObject/GameObject.h"
#include "../Game/Scene/Camera.h"
#include "../Game/GameObject/Animator.h"
#include "../Game/GameObject/EquipmentObject.h"
#include "../MeshLoader/Loader/TerrainLoader.h"
#include "../Game/Scene/Player.h"
#include "../ServerLib/PacketHandler.h"
#include "../Utility/IntervalTimer.h"
#include "../UI/Inventory.h"
#include "../UI/HealthBar.h"
#include "../UI/Profile.h"

class Scene {
public:
	Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList ,std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>, std::shared_ptr<Canvas>> managers, DefaultBufferCPUIterator mainCameraBufferLocation); 
	~Scene();
public:
	void ProcessNetwork(); 
	void Update();
	void SendNetwork(); 

private:
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList); 
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildAniamtionController(); 
	void BuildTerrainBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

	void BuildEnvironment(const std::filesystem::path& envFile);

	void BuildBaseAnimationController();
	void BuildArcherAnimationController();
	void BuildSwordManAnimationController();
	void BuildMageAnimationController();
	void BuildShieldManController();

	void BuildMonsterType1AnimationController();
	void BuildDemonAnimationController(); 
private:
	void ProcessPackets(const uint8_t* buffer, size_t size); 
	const uint8_t* ProcessPacket(const uint8_t* buffer);


	void ProcessPacketProtocolVersion(const uint8_t* buffer);
	void ProcessNotifyId(const uint8_t* buffer);
	void ProcessPlayerExit(const uint8_t* buffer);
	void ProcessLatency(const uint8_t* buffer);
	void ProcessObjectAppeared(const uint8_t* buffer);
	void ProcessObjectDisappeared(const uint8_t* buffer);
	void ProcessObjectRemoved(const uint8_t* buffer);
	void ProcessObjectMove(const uint8_t* buffer);
	void ProcessObjectAttacked(const uint8_t* buffer);
	void ProcessPacketAnimation(const uint8_t* buffer);
	void ProcessGemInteraction(const uint8_t* buffer);
	void ProcessGemCancelInteraction(const uint8_t* buffer);
	void ProcessGemDestroyed(const uint8_t* buffer);
	void ProcessUseItem(const uint8_t* buffer);
	void ProcessAcquiredItem(const uint8_t* buffer);
	void ProcessFireProjectile(const uint8_t* buffer);
	void ProcessProjectileMove(const uint8_t* buffer);

private:
	std::shared_ptr<TextureManager> mTextureManager{ nullptr };
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{ nullptr };
	std::shared_ptr<MaterialManager> mMaterialManager{ nullptr };
	std::shared_ptr<ParticleManager> mParticleManager{ nullptr };
	std::shared_ptr<Canvas> mCanvas{ nullptr };

	std::unordered_map<std::string, Collider> mColliderMap{};
	std::unordered_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	std::unordered_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};

	Camera mCamera{};
	std::unique_ptr<CameraMode> mCameraMode{ nullptr };


	std::unordered_map<NetworkObjectIdType, GameObject*> mGameObjectMap{};
	std::vector<GameObject> mGameObjects{};

	std::vector<GameObject> mEnvironmentObjects{};

	int mNetworkSign{};
	int mInputSign{}; 
	std::vector<DirectX::Keyboard::Keys> mSendKeyList{};

	std::unordered_map<NetworkObjectIdType, Player*> mPlayerIndexmap{};
	std::array<Player, 5> mPlayers{ Player{}, };
	
	Player* mMyPlayer{ nullptr };

	
	
	std::unordered_map<std::string, EquipmentObject> mEquipments{}; 

	AnimatorGraph::BoneMaskAnimationGraphController mBaseAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mArcherAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mSwordManAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mMageAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mShieldManController{};

	AnimatorGraph::AnimationGraphController mMonsterAnimationController{}; 
	AnimatorGraph::AnimationGraphController mDemonAnimationController{};

	GameObject mSkyBox{};
	GameObject mSkyFog{};

	TerrainLoader tLoader{}; 
	TerrainCollider tCollider{};

	DefaultBuffer mTerrainHeaderBuffer{};
	DefaultBuffer mTerrainDataBuffer{};

	Particle test{};
	Particle test1{};
	Particle test2{}; 

	//TextBlock* mNetworkInfoText{ TextBlockManager::GetInstance().CreateTextBlock(L"",D2D1_RECT_F{100.f,0.f,800.f,100.f},StringColor::Black, "NotoSansKR") };
	
	IntervalTimer mIntervalTimer{};

	std::unordered_map<std::string, std::vector<double>> mAnimationTimeMap{};

	Inventory mInventoryUI{};
	HealthBar mHealthBarUI{};
	Profile mProfileUI{};
};