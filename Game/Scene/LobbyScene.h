#pragma once 
#include "../Renderer/Manager/RenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../Game/GameObject/GameObject.h"
#include "../Game/GameObject/EquipmentObject.h"
#include "../Game/Scene/Player.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#include "../Game/UI/Image.h"
#include "../ServerLib/PacketHandler.h"

class LobbyScene : public IScene {
public:
	LobbyScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation);
	virtual ~LobbyScene();
public:
	virtual void Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) override;
	virtual void ProcessNetwork() override;
	virtual void Update() override;
	virtual void SendNetwork() override;
	virtual void Exit() override;
private:
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);

	void BuildLobbyObject(); 

	void BuildBaseMan();
	void BuildSwordMan();
	void BuildArcher();
	void BuildMage();
	void BuildShieldMan();

	void BuildDemon();

	void BuildEquipmentObject();
	void BuildPlayerPrefab();

	void BuildPlayerNameTextBlock();
	void BuildPlayerReadyImage(); 

	void ProcessPackets(const uint8_t* buffer, size_t size);
	const uint8_t* ProcessPacket(const uint8_t* buffer);
private:
	std::shared_ptr<RenderManager> mRenderManager{};

	Camera mCamera{};

	GameObject mSkyBox{};

	std::vector<GameObject> mLobbyProps{}; 

	std::unordered_map<NetworkObjectIdType, std::tuple<Player, TextBlock*, Image, bool>*> mPlayerIndexmap{};
	std::unordered_map<NetworkObjectIdType, UINT> mPlayerSlotMap{};
	std::array<std::tuple<Player,TextBlock*, Image, bool>, 6> mPlayers{};
	
	bool mLookingDemon{ 0 };
	PlayerRole mPlayerRole{ PlayerRole_None };
	UINT mMySlot{ 0 }; 

	bool mCameraRotating{ false };

	std::unordered_map<std::string, EquipmentObject> mEquipments{};
	std::unordered_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	std::unordered_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};
	
	std::unordered_map<std::string, Player> mPlayerPreFabs{}; 

	AnimatorGraph::AnimationGraphController mBaseManAnimationController{};
	AnimatorGraph::AnimationGraphController mSwordManAnimationController{};
	AnimatorGraph::AnimationGraphController mArcherAnimationController{};
	AnimatorGraph::AnimationGraphController mMageAnimationController{};
	AnimatorGraph::AnimationGraphController mShieldManAnimationController{};

	AnimatorGraph::AnimationGraphController mDemonAnimationController{};

	bool mIsReady{ false }; 
};