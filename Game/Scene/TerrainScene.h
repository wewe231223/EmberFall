#pragma once 
#include <array>
#include "../Renderer/Manager/RenderManager.h"

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

class TerrainScene : public IScene {
public:
	TerrainScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation); 
	virtual ~TerrainScene();

public:
	virtual void Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) override; 

	virtual void ProcessNetwork(); 
	virtual void Update();
	virtual void SendNetwork(); 

private:
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList); 
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildAniamtionController(); 

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
	std::shared_ptr<RenderManager> mRenderManager{};

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

	IntervalTimer mIntervalTimer{};

	Inventory mInventoryUI{};
	HealthBar mHealthBarUI{};
	Profile mProfileUI{};
};