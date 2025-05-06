#include "pch.h"
#include "LobbyScene.h"
#include "../MeshLoader/Loader/MeshLoader.h"

// 역할군 전환 버튼 ( 키 ) 를 통해 역할군을 전환 가능 
// 순서는 : 대검 -> 방해 -> 활 -> 마법사 순서로 전환
// 내 캐릭터는 인간 진영에서 0번 슬롯으로 함. ( 모든 클라이언트에서 ) 
// 다른 플레이어는 로비 씬에 입장한 순서대로 1,2,3,4 번 ( 인간 ) 슬롯에 들어감. 

LobbyScene::LobbyScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation) {
	mRenderManager = renderMgr;

	mCamera = Camera(mainCamLocation);
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 0.f, 2.f, 0.f };
	cameraTransform.Look({ 1.f,0.f,0.f });
}

LobbyScene::~LobbyScene() {

}

const uint8_t* LobbyScene::ProcessPacket(const uint8_t* buffer) {

	decltype(auto) header = FbsPacketFactory::GetHeaderPtrSC(buffer);

	auto FindNextPlayerLoc = [this]() {
		for (auto iter = mPlayers.begin(); iter != mPlayers.end(); ++iter) {
			if (not std::get<0>(*iter).GetActiveState()) {
				return iter;
			}
		}
		return mPlayers.end();
		};

	switch (header->type) {
	case Packets::PacketTypes_PT_NOTIFY_ID_SC:
	{
		decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::NotifyIdSC>(buffer);
		gClientCore->InitSessionId(data->playerId());

		break;
	}
	case Packets::PacketTypes_PT_PROTOCOL_VERSION_SC: 
	{
		decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ProtocolVersionSC>(buffer);
		if (PROTOCOL_VERSION_MAJOR != data->major() or
			PROTOCOL_VERSION_MINOR != data->minor()) {
			gClientCore->CloseSession();
			MessageBox(nullptr, L"ERROR!!!!!\nProtocolVersion Mismatching", L"", MB_OK | MB_ICONERROR);
			::exit(0);
		}
		break;
	}
	case Packets::PacketTypes_PT_CONFIRM_SELECTION_ROLE_SC:
	{
		switch (mPlayerRole) {
		case PlayerRole_SwordMan:
		{
			if (mPlayerIndexmap[gClientCore->GetSessionId()] == &mPlayers[5]) {
				mPlayerIndexmap[gClientCore->GetSessionId()] = &mPlayers[mMySlot];

				std::get<1>(mPlayers[mMySlot]).GetName() = std::get<1>(mPlayers[5]).GetName();

				std::get<0>(mPlayers[5]).SetActiveState(false);
				std::get<1>(mPlayers[5]).SetActiveState(false);

				std::get<0>(mPlayers[mMySlot]).SetActiveState(true);

				if (mLookingDemon) {
					mLookingDemon = false;
					mCameraRotating = true;
				}
			}

			auto transform = std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])) = mPlayerPreFabs["SwordMan"].Clone();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform() = transform;
		}
			break;
		case PlayerRole_Archer:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])) = mPlayerPreFabs["Archer"].Clone();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform() = transform;
		}
			break;
		case PlayerRole_Mage:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])) = mPlayerPreFabs["Mage"].Clone();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform() = transform;
		}
			break;
		case PlayerRole_ShieldMan:
		{
			if (mPlayerIndexmap[gClientCore->GetSessionId()] == &mPlayers[5]) {
				mPlayerIndexmap[gClientCore->GetSessionId()] = &mPlayers[mMySlot];

				std::get<1>(mPlayers[mMySlot]).GetName() = std::get<1>(mPlayers[5]).GetName();

				std::get<0>(mPlayers[5]).SetActiveState(false);
				std::get<1>(mPlayers[5]).SetActiveState(false);

				std::get<0>(mPlayers[mMySlot]).SetActiveState(true);

				if (mLookingDemon) {
					mLookingDemon = false;
					mCameraRotating = true;
				}
			}

			auto transform = std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])) = mPlayerPreFabs["ShieldMan"].Clone();
			std::get<0>(*(mPlayerIndexmap[gClientCore->GetSessionId()])).GetTransform() = transform;
		}
			break;
		case PlayerRole_Demon:
		{
			mPlayerIndexmap[gClientCore->GetSessionId()] = &mPlayers[5];
			std::get<1>(mPlayers[5]).GetName() = std::get<1>(mPlayers[mMySlot]).GetName();

			std::get<0>(mPlayers[mMySlot]).SetActiveState(false);
			std::get<0>(mPlayers[5]).SetActiveState(true);

			for (auto i = 0; i < mPlayers.size() - 1; ++i) {
				std::get<1>(mPlayers[i]).SetActiveState(false);
				std::get<2>(mPlayers[i]).SetActiveState(false);
			}

			mLookingDemon = true;
			mCameraRotating = true;
		}
			break;
		}
	}
		break;
	case Packets::PacketTypes_PT_PLAYER_CHANGE_ROLE_SC:
	{
		decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::PlayerChangeRoleSC>(buffer);

		switch (data->role()) {
		case Packets::PlayerRole::PlayerRole_HUMAN_LONGSWORD:
		{
			if (mPlayerIndexmap[data->playerId()] == &mPlayers[5]) {

				auto slot = mPlayerSlotMap[data->playerId()];
				mPlayerIndexmap[data->playerId()] = &mPlayers[slot];
				std::get<1>(mPlayers[slot]).GetName() = std::get<1>(mPlayers[5]).GetName();

				std::get<0>(mPlayers[5]).SetActiveState(false);
				std::get<1>(mPlayers[5]).SetActiveState(false);

				
				std::get<0>(mPlayers[slot]).SetActiveState(true);
				std::get<1>(mPlayers[slot]).SetActiveState(true);
			}


			auto transform = std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])) = mPlayerPreFabs["SwordMan"].Clone();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_HUMAN_ARCHER:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])) = mPlayerPreFabs["Archer"].Clone();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_HUMAN_MAGICIAN:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])) = mPlayerPreFabs["Mage"].Clone();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_HUMAN_SWORD:
		{
			if (mPlayerIndexmap[data->playerId()] == &mPlayers[5]) {

				auto slot = mPlayerSlotMap[data->playerId()];
				mPlayerIndexmap[data->playerId()] = &mPlayers[slot];
				std::get<1>(mPlayers[slot]).GetName() = std::get<1>(mPlayers[5]).GetName();

				std::get<0>(mPlayers[5]).SetActiveState(false);
				std::get<1>(mPlayers[5]).SetActiveState(false);


				std::get<0>(mPlayers[slot]).SetActiveState(true);
				std::get<1>(mPlayers[slot]).SetActiveState(true);
			}

			auto transform = std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])) = mPlayerPreFabs["ShieldMan"].Clone();
			std::get<0>(*(mPlayerIndexmap[data->playerId()])).GetTransform() = transform;

		}
		break;
		case Packets::PlayerRole::PlayerRole_BOSS:
		{
			std::get<0>(*mPlayerIndexmap[data->playerId()]).SetActiveState(false);
			std::get<1>(*mPlayerIndexmap[data->playerId()]).SetActiveState(false);
			std::get<1>(mPlayers[5]).GetName() = std::get<1>(*mPlayerIndexmap[data->playerId()]).GetName();

			mPlayerIndexmap[data->playerId()] = &mPlayers[5];

			std::get<0>(*mPlayerIndexmap[data->playerId()]).SetActiveState(true);
		}
		break;
		default:
			break;
		}
	}
	break; 
	case Packets::PacketTypes_PT_PLAYER_READY_IN_LOBBY_SC:
	{
		decltype(auto) packet = FbsPacketFactory::GetDataPtrSC<Packets::PlayerReadyInLobbySC>(buffer);

		auto id = packet->playerId(); 
		std::get<2>(*(mPlayerIndexmap[packet->playerId()])).SetActiveState(true);
		std::get<3>(*(mPlayerIndexmap[packet->playerId()])) = true; 
		

		break;
	}
	case Packets::PacketTypes_PT_PLAYER_ENTER_IN_LOBBY_SC:
	{
		decltype(auto) packet = FbsPacketFactory::GetDataPtrSC<Packets::PlayerEnterInLobbySC>(buffer);

		mPlayerIndexmap[packet->playerId()] = &(mPlayers[packet->playerSlot()]);
		std::get<0>(*(mPlayerIndexmap[packet->playerId()])).SetActiveState(true);
		if (not mLookingDemon) {
			std::get<1>(*(mPlayerIndexmap[packet->playerId()])).SetActiveState(true);
		}

		std::string name{ flatbuffers::GetCstring(packet->name()) };
		std::get<1>(*(mPlayerIndexmap[packet->playerId()])).GetName() = std::wstring( ConvertUtf8ToWstring(name.c_str()) );

		mPlayerSlotMap[packet->playerId()] = packet->playerSlot();

		if (gClientCore->GetSessionId() == packet->playerId()) {
			mMySlot = packet->playerSlot();
			break; 
		}

		auto role = packet->role();
		switch (role) {
		case Packets::PlayerRole::PlayerRole_HUMAN_LONGSWORD:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])) = mPlayerPreFabs["SwordMan"].Clone();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_HUMAN_ARCHER:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])) = mPlayerPreFabs["Archer"].Clone();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_HUMAN_MAGICIAN:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])) = mPlayerPreFabs["Mage"].Clone();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_HUMAN_SWORD:
		{
			auto transform = std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])) = mPlayerPreFabs["ShieldMan"].Clone();
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])).GetTransform() = transform;
		}
		break;
		case Packets::PlayerRole::PlayerRole_BOSS:
		{
			std::get<0>(*mPlayerIndexmap[packet->playerId()]).SetActiveState(false);
			std::get<1>(*mPlayerIndexmap[packet->playerId()]).SetActiveState(false);
			std::get<1>(mPlayers[5]).GetName() = std::get<1>(*mPlayerIndexmap[packet->playerId()]).GetName();

			mPlayerIndexmap[packet->playerId()] = &mPlayers[5];

			std::get<0>(*mPlayerIndexmap[packet->playerId()]).SetActiveState(true);
		}
		break;
		default:
			break;
		}

		break;
	}
	case Packets::PacketTypes_PT_PLAYER_EXIT_SC:
	{
		decltype(auto) packet = FbsPacketFactory::GetDataPtrSC<Packets::PlayerExitSC>(buffer);

		if (mPlayerIndexmap.contains(packet->playerId())) {
			std::get<0>(*(mPlayerIndexmap[packet->playerId()])).SetActiveState(false);
			std::get<1>(*(mPlayerIndexmap[packet->playerId()])).SetActiveState(false);
			std::get<2>(*(mPlayerIndexmap[packet->playerId()])).SetActiveState(false);
		}

		break;
	}
	// 씬 전환 패킷 처리..
	case Packets::PacketTypes_PT_CHANGE_TO_NEXT_SCENE_SC: 
	{
		decltype(auto) packet = FbsPacketFactory::GetDataPtrSC<Packets::ChangeToNextSceneSC>(buffer);
		PostMessage(mRenderManager->GetWindowHandle(), WM_ADVANCESCENE, 0, 0); 
	}
	break; 
	default:
		break;
	}

	return buffer + header->size;
}


void LobbyScene::Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	LobbyScene::BuildShader(device);
	LobbyScene::BuildMesh(device, commandList);
	LobbyScene::BuildMaterial();
	LobbyScene::BuildLobbyObject(); 
	LobbyScene::BuildBaseMan(); 
	LobbyScene::BuildSwordMan();
	LobbyScene::BuildArcher();
	LobbyScene::BuildMage();
	LobbyScene::BuildShieldMan();
	LobbyScene::BuildDemon();
	LobbyScene::BuildEquipmentObject();
	LobbyScene::BuildPlayerPrefab();
	LobbyScene::BuildPlayerNameTextBlock(); 
	LobbyScene::BuildPlayerReadyImage();

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SkyBoxMaterial");

	mRenderManager->GetLightingManager().ClearLight(commandList);

	std::get<0>(mPlayers[0]) = mPlayerPreFabs["BaseMan"].Clone();
	std::get<0>(mPlayers[1]) = mPlayerPreFabs["BaseMan"].Clone();
	std::get<0>(mPlayers[2]) = mPlayerPreFabs["BaseMan"].Clone();
	std::get<0>(mPlayers[3]) = mPlayerPreFabs["BaseMan"].Clone();
	std::get<0>(mPlayers[4]) = mPlayerPreFabs["BaseMan"].Clone();

	constexpr float Interval = 2.f;
	std::get<0>(mPlayers[0]).GetTransform().SetPosition({ -2.f * Interval, 0.f, 5.f });
	std::get<0>(mPlayers[1]).GetTransform().SetPosition({ -1.f * Interval, 0.f, 5.f });
	std::get<0>(mPlayers[2]).GetTransform().SetPosition({ 0.f * Interval, 0.f, 5.f });
	std::get<0>(mPlayers[3]).GetTransform().SetPosition({ 1.f * Interval, 0.f, 5.f });
	std::get<0>(mPlayers[4]).GetTransform().SetPosition({ 2.f * Interval, 0.f, 5.f });

	std::get<0>(mPlayers[5]) = mPlayerPreFabs["Demon"].Clone();
	std::get<0>(mPlayers[5]).GetTransform().SetPosition({ 0.f, 0.f, -10.f });
	std::get<0>(mPlayers[5]).GetTransform().Rotate(0.f, 110.f, 0.f);

	std::get<0>(mPlayers[0]).SetActiveState(false);
	std::get<0>(mPlayers[1]).SetActiveState(false);
	std::get<0>(mPlayers[2]).SetActiveState(false);
	std::get<0>(mPlayers[3]).SetActiveState(false);
	std::get<0>(mPlayers[4]).SetActiveState(false);
	std::get<0>(mPlayers[5]).SetActiveState(false);

	mCamera.GetTransform().Look({ 0.f,2.f, 1.f });

	mRenderManager->GetLightingManager().ClearLight(commandList);
	auto& light = mRenderManager->GetLightingManager().GetLight(0);
	light.mType = LightType::Directional;
	light.Direction = { 1.f, 3.f, 1.f };
	light.Diffuse = { 1.f, 1.f, 1.f, 1.f };
	light.Specular = { 1.f, 1.f, 1.f, 1.f };
	light.Ambient = { 0.2f, 0.2f, 0.2f, 1.f };

	auto packet = FbsPacketFactory::PlayerEnterInLobbyCS(gClientCore->GetSessionId()); 
	gClientCore->Send(packet); 


	mReadyButton = Button{};
	mReadyButton.Init(mRenderManager->GetCanvas(), Button::InvokeCondition::LeftClick, mRenderManager->GetTextureManager().GetTexture("Left"));
	mReadyButton.SetRect(0.f, 0.f, 100.f, 100.f);


}

void LobbyScene::ProcessNetwork() {
	auto packetHandler = gClientCore->GetPacketHandler();
	decltype(auto) buffer = packetHandler->GetBuffer();

	LobbyScene::ProcessPackets(reinterpret_cast<const uint8_t*>(buffer.Data()), buffer.Size());
}

void LobbyScene::Update() {

	mReadyButton.Update(); 

	PlayerRole prevRole = mPlayerRole;


	if (not mIsReady and not mCameraRotating) {
		if (Input.GetKeyboardTracker().pressed.Left) {
			if (mPlayerRole == PlayerRole_None) mPlayerRole = PlayerRole_Demon;
			else mPlayerRole = static_cast<PlayerRole>(PlayerRole_SwordMan + ((mPlayerRole - PlayerRole_SwordMan + (PlayerRole_END - PlayerRole_SwordMan) - 1) % (PlayerRole_END - PlayerRole_SwordMan)));
		}

		if (Input.GetKeyboardTracker().pressed.Right) {
			if (mPlayerRole == PlayerRole_None) mPlayerRole = PlayerRole_SwordMan;
			else mPlayerRole = static_cast<PlayerRole>(PlayerRole_SwordMan + ((mPlayerRole - PlayerRole_SwordMan + 1) % (PlayerRole_END - PlayerRole_SwordMan)));
		}


		if (prevRole != mPlayerRole) {
			Packets::PlayerRole role = Packets::PlayerRole::PlayerRole_NONE;
			switch (mPlayerRole) {
			case PlayerRole_SwordMan:
				role = Packets::PlayerRole::PlayerRole_HUMAN_LONGSWORD;
				break;
			case PlayerRole_Archer:
				role = Packets::PlayerRole::PlayerRole_HUMAN_ARCHER;
				break;
			case PlayerRole_Mage:
				role = Packets::PlayerRole::PlayerRole_HUMAN_MAGICIAN;
				break;
			case PlayerRole_ShieldMan:
				role = Packets::PlayerRole::PlayerRole_HUMAN_SWORD;
				break;
			case PlayerRole_Demon:
				role = Packets::PlayerRole::PlayerRole_BOSS;
				break;
			default:
				break;
			}
			decltype(auto) packet = FbsPacketFactory::PlayerSelectRole(gClientCore->GetSessionId(), role);
			gClientCore->Send(packet); 
		}
	}


	if (Input.GetKeyboardTracker().pressed.Tab) {
		if (not mCameraRotating) {
			if (mLookingDemon) { // 인간 진영 선택 
				mLookingDemon = false;
				std::get<1>(mPlayers[5]).SetActiveState(false);
				std::get<2>(mPlayers[5]).SetActiveState(false); 
				mCameraRotating = true; 
			}
			else { // 악마 진영 선택 
				mLookingDemon = true;
				for (auto i = 0; i < mPlayers.size() - 1; ++i) {
					std::get<1>(mPlayers[i]).SetActiveState(false);
					std::get<2>(mPlayers[i]).SetActiveState(false);
				}
				mCameraRotating = true;
			}
		}
	}

	if (Input.GetKeyboardTracker().pressed.Enter and not mIsReady and mPlayerRole != PlayerRole_None) {
		mIsReady = true; 

		decltype(auto) packet = FbsPacketFactory::PlayerReadyInLobbyCS(gClientCore->GetSessionId());
		gClientCore->Send(packet);
	}

	if (mCameraRotating) {
		mCamera.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(180.f) * Time.GetDeltaTime<float>(), 0.f); 
		
		auto& rot = mCamera.GetTransform().GetRotation();

		auto euler = rot.ToEuler();

		if (not mLookingDemon and std::fabs(euler.y) - 0.01f <= 0.f) {
			mCamera.GetTransform().SetRotation(DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(0.f, 0.f, 0.f));
			mCameraRotating = false;

			for (auto i = 0; i < mPlayers.size() - 1; ++i) {
				if (std::get<0>(mPlayers[i]).GetActiveState()) {
					std::get<1>(mPlayers[i]).SetActiveState(true);
					if (std::get<3>(mPlayers[i])) {
						std::get<2>(mPlayers[i]).SetActiveState(true);
					}
				}
			}

		}

		if (mLookingDemon and std::fabs(std::fabs(euler.y) - DirectX::XM_PI) <= 0.05f) {
			mCamera.GetTransform().SetRotation(DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(180.f), 0.f, 0.f));
			mCameraRotating = false;

			if (std::get<0>(mPlayers[5]).GetActiveState()) {
				std::get<1>(mPlayers[5]).SetActiveState(true);
				if (std::get<3>(mPlayers[5])) {
					std::get<2>(mPlayers[5]).SetActiveState(true);
				}
			}
		}

	}


	mCamera.UpdateBuffer(); 

	for (auto& object : mLobbyProps) {
		object.UpdateShaderVariables();
		auto [mesh, shader, modelContext] = object.GetRenderData();
		mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(shader, mesh, modelContext);
	}

	for (auto& player : mPlayers) {
		if (std::get<0>(player).GetActiveState() == false) {
			continue;
		}
		std::get<0>(player).Update(mRenderManager->GetMeshRenderManager());
		std::get<1>(player).Update();
		std::get<2>(player).Update(); 
	}

	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition(); 
	mSkyBox.UpdateShaderVariables();

	mSkyBox.UpdateShaderVariables();
	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);
}

void LobbyScene::SendNetwork() {
	if (Input.GetKeyboardTracker().pressed.Enter and not mIsReady) {
		decltype(auto) packet = FbsPacketFactory::PlayerReadyInLobbyCS(gClientCore->GetSessionId());
		gClientCore->Send(packet);
	}
}

void LobbyScene::Exit() {
	for (auto& p : mPlayers) {
		std::get<1>(p).SetActiveState(false);
	}
}

void LobbyScene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 100.f);

	mMeshMap["Plane"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Plane, 50);

	MeshLoader Loader{};
	MeshData data{};

	data = Loader.Load("Resources/Assets/Knight/LongSword/SwordMan.glb");
	mMeshMap["SwordMan"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Knight/BaseAnim/BaseAnim.gltf");
	mMeshMap["HumanBase"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/Demon.glb");
	mMeshMap["Demon"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/sword/LongSword.glb");
	mMeshMap["Sword"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/great_sword/Sword.glb");
	mMeshMap["GreatSword"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Bow/Bow.glb");
	mMeshMap["Bow"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Bow/Arrow.glb");
	mMeshMap["Arrow"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Bow/quiver.glb");
	mMeshMap["Quiver"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Weapon/Shield/Shield.glb");
	mMeshMap["Shield"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/DemonWeapon.glb");
	mMeshMap["DemonWeapon"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/DemonCloth.glb");
	mMeshMap["DemonCloth"] = std::make_unique<Mesh>(device, commandList, data);
}

void LobbyScene::BuildMaterial() {
	MaterialConstants mat{};

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Front_0");
	mat.mDiffuseTexture[1] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Back_0");
	mat.mDiffuseTexture[2] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Top_0");
	mat.mDiffuseTexture[3] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Bottom_0");
	mat.mDiffuseTexture[4] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Left_0");
	mat.mDiffuseTexture[5] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Right_0");
	mRenderManager->GetMaterialManager().CreateMaterial("SkyBoxMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("dirt_2");
	mRenderManager->GetMaterialManager().CreateMaterial("GroundMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("Paladin_diffuse");
	mRenderManager->GetMaterialManager().CreateMaterial("HumanMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("SwordA_v004_Default_AlbedoTransparency");
	mRenderManager->GetMaterialManager().CreateMaterial("SwordMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("sword_base");
	mRenderManager->GetMaterialManager().CreateMaterial("GreatSwordMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("Bow_DIFF");
	mRenderManager->GetMaterialManager().CreateMaterial("BowMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("Quiver_baseColor");
	mRenderManager->GetMaterialManager().CreateMaterial("QuiverMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("T_BigDemonWarrior_Body_Albedo_Skin_3");
	mRenderManager->GetMaterialManager().CreateMaterial("DemonMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("T_BigDemonWarrior_Axe_Albedo_Skin_1");
	mRenderManager->GetMaterialManager().CreateMaterial("DemonWeaponMaterial", mat);

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("T_BigDemonWarrior_Clothes_Albedo_Skin_1");
	mRenderManager->GetMaterialManager().CreateMaterial("DemonClothMaterial", mat);
}


void LobbyScene::BuildShader(ComPtr<ID3D12Device> device) {
	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<SkyBoxShader>();
	shader->CreateShader(device);
	mShaderMap["SkyBoxShader"] = std::move(shader);

	shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);
	mShaderMap["StandardShader"] = std::move(shader);

	shader = std::make_unique<SkinnedShader>();
	shader->CreateShader(device);
	mShaderMap["SkinnedShader"] = std::move(shader);
}

void LobbyScene::BuildLobbyObject() {
	{
		auto& object = mLobbyProps.emplace_back();
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["Plane"].get();
		object.mMaterial = mRenderManager->GetMaterialManager().GetMaterial("GroundMaterial");

		object.SetActiveState(true);
	}
}

void LobbyScene::BuildBaseMan() {
	mAnimationMap["BaseAnim"].Load("Resources/Assets/Knight/BaseAnim/BaseAnim.gltf");
	auto& loader = mAnimationMap["BaseAnim"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mBaseManAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildSwordMan() {
	mAnimationMap["LongSword"].Load("Resources/Assets/Knight/LongSword/SwordMan.glb");
	auto& loader = mAnimationMap["LongSword"];

	AnimatorGraph::AnimationState idle{};
	 
	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle"; 

	mSwordManAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildArcher() {
	mAnimationMap["Archer"].Load("Resources/Assets/Knight/Archer/Archer.glb");
	auto& loader = mAnimationMap["Archer"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mArcherAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildMage() {
	mAnimationMap["Magician"].Load("Resources/Assets/Knight/Mage/Magician.glb");
	auto& loader = mAnimationMap["Magician"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mMageAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildShieldMan() {
	mAnimationMap["ShieldMan"].Load("Resources/Assets/Knight/ShieldMan/ShieldMan.glb");
	auto& loader = mAnimationMap["ShieldMan"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mShieldManAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildDemon() {
	mAnimationMap["Demon"].Load("Resources/Assets/Demon/Demon.glb");
	auto& loader = mAnimationMap["Demon"];

	AnimatorGraph::AnimationState idle{};

	idle.clip = loader.GetClip(0);
	idle.loop = true;
	idle.name = "Idle";

	mDemonAnimationController = AnimatorGraph::AnimationGraphController({ idle });
}

void LobbyScene::BuildEquipmentObject() {
	{
		mEquipments["Sword"] = EquipmentObject{};
		mEquipments["Sword"].mMesh = mMeshMap["Sword"].get();
		mEquipments["Sword"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Sword"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SwordMaterial");
		mEquipments["Sword"].mEquipJointIndex = 36;
		mEquipments["Sword"].SetActiveState(true);
	}


	{
		mEquipments["GreatSword"] = EquipmentObject{};
		mEquipments["GreatSword"].mMesh = mMeshMap["GreatSword"].get();
		mEquipments["GreatSword"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["GreatSword"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("GreatSwordMaterial");
		mEquipments["GreatSword"].mEquipJointIndex = 36;
		mEquipments["GreatSword"].SetActiveState(true);
	}

	{
		mEquipments["Bow"] = EquipmentObject{};
		mEquipments["Bow"].mMesh = mMeshMap["Bow"].get();
		mEquipments["Bow"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Bow"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("BowMaterial");
		mEquipments["Bow"].mEquipJointIndex = 12;
		mEquipments["Bow"].SetActiveState(true);
	}

	{
		mEquipments["Quiver"] = EquipmentObject{};
		mEquipments["Quiver"].mMesh = mMeshMap["Quiver"].get();
		mEquipments["Quiver"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Quiver"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("QuiverMaterial");
		mEquipments["Quiver"].mEquipJointIndex = 3;
		mEquipments["Quiver"].SetActiveState(true);
	}

	{
		mEquipments["Shield"] = EquipmentObject{};
		mEquipments["Shield"].mMesh = mMeshMap["Shield"].get();
		mEquipments["Shield"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Shield"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial");
		mEquipments["Shield"].mEquipJointIndex = 11;
		mEquipments["Shield"].SetActiveState(true);
	}

	{
		mEquipments["DemonCloth"] = EquipmentObject{};
		mEquipments["DemonCloth"].mMesh = mMeshMap["DemonCloth"].get();
		mEquipments["DemonCloth"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["DemonCloth"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("DemonClothMaterial");
		mEquipments["DemonCloth"].mEquipJointIndex = 0;
		mEquipments["DemonCloth"].SetActiveState(true);
	}

	{
		mEquipments["DemonWeapon"] = EquipmentObject{};
		mEquipments["DemonWeapon"].mMesh = mMeshMap["DemonWeapon"].get();
		mEquipments["DemonWeapon"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["DemonWeapon"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("DemonWeaponMaterial");
		mEquipments["DemonWeapon"].mEquipJointIndex = 28;
		mEquipments["DemonWeapon"].SetActiveState(true);
	}
}

void LobbyScene::BuildPlayerPrefab() {
	mPlayerPreFabs["BaseMan"] = Player{ mMeshMap["HumanBase"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mBaseManAnimationController };

	mPlayerPreFabs["SwordMan"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mSwordManAnimationController };
	mPlayerPreFabs["SwordMan"].AddEquipment(mEquipments["GreatSword"]);
	
	mPlayerPreFabs["ShieldMan"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mShieldManAnimationController };
	mPlayerPreFabs["ShieldMan"].AddEquipment(mEquipments["Sword"].Clone());
	mPlayerPreFabs["ShieldMan"].AddEquipment(mEquipments["Shield"].Clone());

	mPlayerPreFabs["Archer"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mArcherAnimationController };
	mPlayerPreFabs["Archer"].AddEquipment(mEquipments["Bow"].Clone());
	mPlayerPreFabs["Archer"].AddEquipment(mEquipments["Quiver"].Clone());

	mPlayerPreFabs["Mage"] = Player{ mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("HumanMaterial"), mMageAnimationController };

	mPlayerPreFabs["Demon"] = Player{ mMeshMap["Demon"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("DemonMaterial"), mDemonAnimationController };
	mPlayerPreFabs["Demon"].AddEquipment(mEquipments["DemonWeapon"].Clone());
	mPlayerPreFabs["Demon"].AddEquipment(mEquipments["DemonCloth"].Clone());
}

void LobbyScene::BuildPlayerNameTextBlock() {
	const float xInterval = 380.f;
	const float xPadding = 150.f;
	const float namePlateWidth = 240.f; // 최대 너비 확보
	const float namePlateHeight = 30.f;
	const float originalWidth = 100.f;
	const float y = 550.f;

	for (auto i = 0; i < mPlayers.size() - 1; ++i) {
		auto& player = mPlayers[i];
		float centerX = xPadding + xInterval * i + originalWidth / 2.f;
		std::get<1>(player) = NamePlate();
		std::get<1>(player).Init(
			mRenderManager->GetCanvas(),
			mRenderManager->GetTextureManager().GetTexture("mid_dark_bar"),
			centerX - namePlateWidth / 2.f, y,
			namePlateWidth, namePlateHeight);
		std::get<1>(player).GetName() = L"Player" + std::to_wstring(i);
		std::get<1>(player).SetActiveState(false);
	}

	auto& player = mPlayers[5];
	float centerX = 900.f + originalWidth / 2.f;
	std::get<1>(player) = NamePlate();
	std::get<1>(player).Init(
		mRenderManager->GetCanvas(),
		mRenderManager->GetTextureManager().GetTexture("mid_dark_bar"),
		centerX - namePlateWidth / 2.f, 750.f,
		namePlateWidth, namePlateHeight);
	std::get<1>(player).GetName() = L"Player" + std::to_wstring(4);
	std::get<1>(player).SetActiveState(false);
}


void LobbyScene::BuildPlayerReadyImage() {
	std::array<CanvasRect, 6> namePlateRects;

	const float namePlateWidth = 240.f; // 이름표와 일치
	const float readyImageWidth = 30.f;
	const float readyImageHeight = 30.f;
	const float namePlateHeight = 30.f;
	const float xOffsetFromText = 10.f;

	const float xInterval = 380.f;
	const float xPadding = 150.f;
	const float originalWidth = 100.f;

	for (int i = 0; i < 5; ++i) {
		float centerX = xPadding + xInterval * i + originalWidth / 2.f;
		float textLeft = centerX - namePlateWidth / 2.f;
		float textTop = 550.f;

		namePlateRects[i].LTx = textLeft + namePlateWidth + xOffsetFromText;
		namePlateRects[i].LTy = textTop;
		namePlateRects[i].width = readyImageWidth;
		namePlateRects[i].height = readyImageHeight;
	}

	{
		float centerX = 900.f + originalWidth / 2.f;
		float textLeft = centerX - namePlateWidth / 2.f;
		float textTop = 750.f;

		namePlateRects[5].LTx = textLeft + namePlateWidth + xOffsetFromText;
		namePlateRects[5].LTy = textTop;
		namePlateRects[5].width = readyImageWidth;
		namePlateRects[5].height = readyImageHeight;
	}

	for (int i = 0; i < 6; ++i) {
		auto& player = mPlayers[i];
		std::get<2>(player) = Image{};
		std::get<2>(player).Init(
			mRenderManager->GetCanvas(),
			mRenderManager->GetTextureManager().GetTexture("check"));
		std::get<2>(player).GetRect() = namePlateRects[i];
		std::get<2>(player).SetActiveState(false);
	}
}


void LobbyScene::ProcessPackets(const uint8_t* buffer, size_t size) {
	const uint8_t* iter = buffer; 

	while (iter < buffer + size) {
		iter = ProcessPacket(iter);
	}
}

