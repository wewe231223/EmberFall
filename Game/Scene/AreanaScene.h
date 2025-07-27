#pragma once 
#include <array>
#include "../Renderer/Manager/RenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../System/Sound.h"
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
#include "../Game/GameObject/TerrainObject.h"
#include "../External/Include/absl/container/flat_hash_map.h"
#include "../Game/Scene/MaterialLoader.h"
#include "../Game/Scene/LayerIndexMap.h"
#include "../Game/UI/Image.h"


class ArenaScene : public IScene {
	using duration = std::chrono::milliseconds;
public:
	ArenaScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation); 
	virtual ~ArenaScene(); 

public:
	virtual void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) override;

	virtual void ProcessNetwork();
	virtual void Update();
	virtual void SendNetwork();
	virtual void Exit();

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

	void SendLook();
	void UpdateSound();
private:
	void ProcessPackets(const uint8_t* buffer, size_t size);
	const uint8_t* ProcessPacket(const uint8_t* buffer);

	void ProcessPlayerExit(const uint8_t* buffer); 
	void ProcessLatency(const uint8_t* buffer);
	void ProcessObjectAppeared(const uint8_t* buffer);
	void ProcessObjectDisappeared(const uint8_t* buffer);
	void ProcessObjectRemoved(const uint8_t* buffer);
	void ProcessObjectMove(const uint8_t* buffer);
	void ProcessObjectAttacked(const uint8_t* buffer);
	void ProcessPacketAnimation(const uint8_t* buffer);
	void ProcessFireProjectile(const uint8_t* buffer);
	void ProcessProjectileMove(const uint8_t* buffer);
	void ProcessChangeScene(const uint8_t* buffer);
	void ProcessHeartBeat(const uint8_t* buffer);
	void ProcessGameEnd(const uint8_t* buffer);

private:
	std::shared_ptr<RenderManager> mRenderManager{};

	absl::flat_hash_map<std::string, Collider> mColliderMap{};
	absl::flat_hash_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	absl::flat_hash_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};

	Camera mCamera{};
	CameraMode* mCurrentCameraMode{ nullptr };
	std::unique_ptr<CameraMode> mFreeCameraMode{ nullptr };
	std::unique_ptr<CameraMode> mTPPCameraMode{ nullptr };

	absl::flat_hash_map<NetworkObjectIdType, GameObject*> mGameObjectMap{};

	LayerIndexMap mLayerIndexMap{};
	absl::flat_hash_map<NetworkObjectIdType, std::pair<UINT, Sound*>> mSoundMap{};

	std::vector<GameObject> mGameObjects{};
	std::vector<GameObject> mProjectileObjects{};


	std::vector<LODGameObject> mEnvironmentObjects{};

	int mInputSign{};

	std::array<duration, 10> mLatency{};
	UINT mLatencySampleIndex{ 0 };

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

	HealthBar mHealthBarUI{};
	Profile mProfileUI{};

	MaterialFileLoader mMaterialLoader{};
	bool mIsBlind{ false };
	bool mExpired{ false };

	float mAvgLatency{}; 

	Image mGameEnding{};


	float mFireLock{ false };
#ifdef DEV_MODE
	TextBlock* mLatencyBlock{ TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ 1720.f, 50.f, 1920.f, 100.f }, StringColor::BurlyWood, "NotoSansKR") };
#endif
};

template<typename Tu>
inline float ArenaScene::GetAverageLatency() {
	auto sumofSamples = std::accumulate(mLatency.begin(), mLatency.end(), duration::zero(),
		[](const duration& a, const duration& b) {
			if (b.count() <= 0.0)
				return a;
			return a + b;
		});

	return std::chrono::duration_cast<std::chrono::duration<float, typename Tu::period>> (sumofSamples / mLatency.size()).count();
}
