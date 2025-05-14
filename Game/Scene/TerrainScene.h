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
#include "../External/Include/absl/container/flat_hash_map.h"

class TerrainScene : public IScene {
	using duration = std::chrono::milliseconds; 
public:
	TerrainScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation); 
	virtual ~TerrainScene();

public:
	virtual void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) override; 

	virtual void ProcessNetwork(); 
	virtual void Update();
	virtual void SendNetwork(); 
	virtual void Exit();

	void SendLook(); 
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

	template<typename Tu> 
	float GetAverageLatency(); 
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
	void ProcessChangeScene(const uint8_t* buffer);
	void ProcessBuffHeal(const uint8_t* buffer);
	void ProcessHeartBeat(const uint8_t* buffer);
private:
	std::shared_ptr<RenderManager> mRenderManager{};

	absl::flat_hash_map<std::string, Collider> mColliderMap{};
	absl::flat_hash_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	absl::flat_hash_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};

	Camera mCamera{};
	std::unique_ptr<CameraMode> mCameraMode{ nullptr };

	absl::flat_hash_map<NetworkObjectIdType, GameObject*> mGameObjectMap{};
	std::vector<GameObject> mGameObjects{};
	std::vector<GameObject> mItemObjects{}; 

	std::vector<GameObject> mEnvironmentObjects{};

	int mNetworkSign{};
	int mInputSign{}; 

	std::array<duration, 10> mLatency{}; 
	UINT mLatencySampleIndex{ 0 };
#ifdef DEV_MODE
	TextBlock* mLatencyBlock{ TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ 1720.f, 50.f, 1920.f, 100.f }, StringColor::BurlyWood, "NotoSansKR") };
	TextBlock* mPktsBlock{ TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ 1720.f, 70.f, 1920.f, 120.f }, StringColor::BurlyWood, "NotoSansKR") };
#endif 

	absl::flat_hash_map<NetworkObjectIdType, Player*> mPlayerIndexmap{};
	std::array<Player, 5> mPlayers{ Player{}, };
	
	Player* mMyPlayer{ nullptr };

	absl::flat_hash_map<std::string, EquipmentObject> mEquipments{};

	absl::flat_hash_map<NetworkObjectIdType, Particle> mParticleMap{};

	AnimatorGraph::BoneMaskAnimationGraphController mBaseAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mArcherAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mSwordManAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mMageAnimationController{};
	AnimatorGraph::BoneMaskAnimationGraphController mShieldManController{};

	AnimatorGraph::AnimationGraphController mMonsterAnimationController{}; 
	AnimatorGraph::AnimationGraphController mDemonAnimationController{};

	GameObject mSkyBox{};

	TerrainLoader tLoader{}; 
	TerrainCollider tCollider{};

	DefaultBuffer mTerrainHeaderBuffer{};
	DefaultBuffer mTerrainDataBuffer{};

	

	IntervalTimer mIntervalTimer{};

	Inventory mInventoryUI{};
	HealthBar mHealthBarUI{};
	Profile mProfileUI{};

	float mAvgLatency{ 0.f };

	bool mInitialized{ false }; 
};

template<typename Tu> 
inline float TerrainScene::GetAverageLatency() {
	auto sumofSamples = std::accumulate(mLatency.begin(), mLatency.end(), duration::zero(),
		[](const duration& a, const duration& b) {
			if (b.count() <= 0.0)
				return a;
			return a + b;
		});

	return std::chrono::duration_cast<std::chrono::duration<float, typename Tu::period>> (sumofSamples / mLatency.size()).count();
}
