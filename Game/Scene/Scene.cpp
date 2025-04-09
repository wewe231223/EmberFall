#include <ranges>
#include <random>
#include "pch.h"
#include "Scene.h"
#include "../Renderer/Core/Renderer.h"
#include "../MeshLoader/Loader/MeshLoader.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#ifdef _DEBUG
#pragma comment(lib,"out/debug/MeshLoader.lib")
#else 
#pragma comment(lib,"out/release/MeshLoader.lib")
#endif
#include "../Utility/NonReplacementSampler.h"
#include "../MeshLoader/Loader/TerrainBaker.h"
#include "../ServerLib/GameProtocol.h"

#pragma region PacketProcessFn 
void Scene::ProcessNotifyId(PacketHeader* header) {
	gClientCore->InitSessionId(header->id);
	mNetworkInfoText->GetText() = std::format(L"My Session ID : {}", header->id);
}

void Scene::ProcessPacketProtocolVersion(PacketHeader* header) {
	auto protocolVersion = reinterpret_cast<PacketProtocolVersion*>(header);
	if (PROTOCOL_VERSION_MAJOR != protocolVersion->major or
		PROTOCOL_VERSION_MINOR != protocolVersion->minor) {

		gClientCore->CloseSession();

		MessageBox(nullptr, L"ERROR!!!!!\nProtocolVersion Mismatching", L"", MB_OK | MB_ICONERROR);
		::exit(0);
	}
}	

// Deprecated 
void Scene::ProcessPlayerPacket(PacketHeader* header) {

}

void Scene::ProcessObjectPacket(PacketHeader* header) {
	auto packet = reinterpret_cast<PacketSC::PacketObject*>(header);
	
	// 플레이어 영역에 해당한다면
	if (packet->objId < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(packet->objId)) {
			mPlayerIndexmap[packet->objId]->GetTransform().GetPosition() = packet->position;

			if (packet->objId != gClientCore->GetSessionId()) {
				auto euler = mPlayerIndexmap[packet->objId]->GetTransform().GetRotation().ToEuler();
				euler.y = packet->rotationYaw;
				mPlayerIndexmap[packet->objId]->GetTransform().GetRotation() = SimpleMath::Quaternion::CreateFromYawPitchRoll(euler.y, euler.x, euler.z);
			}

		}
	}
	// 그게 아니라면 기타 게임 오브젝트이다. 
	else {
		if (mGameObjectMap.contains(packet->objId)) {
			mGameObjectMap[packet->objId]->GetTransform().GetPosition() = packet->position;
			auto euler = mGameObjectMap[packet->objId]->GetTransform().GetRotation().ToEuler();
			euler.y = packet->rotationYaw;
			mGameObjectMap[packet->objId]->GetTransform().GetRotation() = SimpleMath::Quaternion::CreateFromYawPitchRoll(euler.y, euler.x, euler.z);
		}

	}
}

void Scene::ProcessObjectDead(PacketHeader* header) {

}

void Scene::ProcessObjectAppeared(PacketHeader* header) {
	auto FindNextPlayerLoc = [this]() {
		for (auto iter = mPlayers.begin(); iter != mPlayers.end(); ++iter) {
			if (not iter->GetActiveState()) {
				return iter;
			}
		}
		return mPlayers.end();
		};

	auto FindNextObjectLoc = [this]() {
		for (auto iter = mGameObjects.begin(); iter != mGameObjects.end(); ++iter) {
			if (not *iter) {
				return iter;
			}
		}
		return mGameObjects.end();
		};


	auto packet = reinterpret_cast<PacketSC::PacketObjectAppeared*>(header);
	// 플레이어 등장 
	if (packet->objId < OBJECT_ID_START) {
		// 내 플레이어 등장 
		if (packet->objId == gClientCore->GetSessionId()) {
			// 플레이어 인스턴스가 없다면 
			if (not mPlayerIndexmap.contains(packet->objId)) {

				auto nextLoc = FindNextPlayerLoc();

				if (nextLoc == mPlayers.end()) {
					Crash("There is no more space for My Player!!");
				}

				*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mMaterialManager->GetMaterial("CubeMaterial"), mSwordManAnimationController);
				mPlayerIndexmap[packet->objId] = &(*nextLoc);
				mMyPlayer = &(*nextLoc);

				mMyPlayer->AddEquipment(mEquipments["Sword"].Clone());
				mMyPlayer->SetMyPlayer();
			
				mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
			    //mCameraMode = std::make_unique<TPPCameraMode>(&mCamera, mMyPlayer->GetTransform(), SimpleMath::Vector3{ 0.f, 1.8f, 3.f });
				mCameraMode->Enter();

				Scene::SetInputBaseAnimMode();


			}
			else {
				if (mPlayerIndexmap[packet->objId] != nullptr) {
					mPlayerIndexmap[packet->objId]->SetActiveState(true);
				}
			}
		}
		// 다른 플레이어 등장 
		else {
			// 그 플레이어 인스턴스가 없다면  
			if (not mPlayerIndexmap.contains(packet->objId)) {
				auto nextLoc = FindNextPlayerLoc();
				if (nextLoc == mPlayers.end()) { 
					Crash("There is no more space for Other Player!!"); 
				}

				*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mMaterialManager->GetMaterial("CubeMaterial"), mSwordManAnimationController);
				mPlayerIndexmap[packet->objId] = &(*nextLoc);

			}
			else {
				if (mPlayerIndexmap[packet->objId] != nullptr) {
					mPlayerIndexmap[packet->objId]->SetActiveState(true);
				}
			}
		}
	}
	// 이외 오브젝트 등장 
	else {
		if (not mGameObjectMap.contains(packet->objId)) {
			auto nextLoc = FindNextObjectLoc();
			if (nextLoc == mGameObjects.end()) {
				Crash("There is no more space for Other Object!!");
			}



			switch (packet->entity) {
			case MONSTER1:
			case MONSTER2:
			case MONSTER3:
			{
				*nextLoc = GameObject{};
				mGameObjectMap[packet->objId] = &(*nextLoc);

				nextLoc->mShader = mShaderMap["SkinnedShader"].get();
				nextLoc->mMesh = mMeshMap["MonsterType1"].get();
				nextLoc->mMaterial = mMaterialManager->GetMaterial("MonsterType1Material");
				nextLoc->mGraphController = mMonsterType1AnimationController;
				nextLoc->mAnimated = true;
				nextLoc->SetActiveState(true);

				nextLoc->GetTransform().Scaling(0.3f, 0.3f, 0.3f);
				nextLoc->GetTransform().SetPosition(packet->position);
			}
				break;
			case CORRUPTED_GEM:
			{
				*nextLoc = GameObject{};
				mGameObjectMap[packet->objId] = &(*nextLoc);
				nextLoc->mShader = mShaderMap["StandardShader"].get();
				nextLoc->mMesh = mMeshMap["CorruptedGem"].get();
				nextLoc->mMaterial = mMaterialManager->GetMaterial("CorruptedGemMaterial");
				nextLoc->SetActiveState(true);

				nextLoc->GetTransform().SetPosition(packet->position);
			}
				break;
			default:
			{
				*nextLoc = GameObject{};
				mGameObjectMap[packet->objId] = &(*nextLoc);
				nextLoc->mShader = mShaderMap["StandardShader"].get();
				nextLoc->mMesh = mMeshMap["Cube"].get();
				nextLoc->mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
				nextLoc->SetActiveState(true);

				nextLoc->GetTransform().SetPosition(packet->position);
			}
				break;
			}
		}
		else {
			if (mGameObjectMap[packet->objId] != nullptr) {
				mGameObjectMap[packet->objId]->SetActiveState(true);
			}
		}

	}
}

void Scene::ProcessObjectDisappeared(PacketHeader* header) {
	auto packet = reinterpret_cast<PacketSC::PacketObjectDisappeared*>(header);

	if (packet->objId < OBJECT_ID_START) {

		if (packet->objId == gClientCore->GetSessionId()) {
			return; 
		}

		if (mPlayerIndexmap.contains(packet->objId)) {
			mPlayerIndexmap[packet->objId]->SetActiveState(false);
		}
	}
	else {
		if (mGameObjectMap.contains(packet->objId)) {
			mGameObjectMap[packet->objId]->SetActiveState(false);
		}
	}
}

void Scene::ProcessPlayerExit(PacketHeader* header) {
	if (mPlayerIndexmap.contains(header->id)) {
		mPlayerIndexmap[header->id]->SetActiveState(false);
		mPlayerIndexmap.erase(header->id);
	}
}

void Scene::ProcessAcquiredItem(PacketHeader* header) {

}

void Scene::ProcessObjectAttacked(PacketHeader* header) {

}

void Scene::ProcessUseItem(PacketHeader* header) {

}

void Scene::ProcessRestoreHP(PacketHeader* header) {

}
void Scene::ProcessPacketAnimation(PacketHeader* header) {
	auto packet = reinterpret_cast<PacketSC::PacketAnimationState*>(header);
	// 플레이어인 경우 
	if (packet->objId < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(packet->objId)) {
			mPlayerIndexmap[packet->objId]->GetBoneMaskController().Transition(static_cast<size_t>(packet->animState), 0.09);
		}
	}
	else {
		
	}

}
#pragma endregion 


Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation) {
	
	mInputSign = NonReplacementSampler::GetInstance().Sample(); 
	mNetworkSign = NonReplacementSampler::GetInstance().Sample();
	
	gClientCore->Init();
	auto res = gClientCore->Start("127.0.0.1", 7777);
	if (false == res) {
		DebugBreak(); 
		Crash(false);
	}


	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);
	mParticleManager = std::get<3>(managers);

	Scene::BuildShader(device); 
	Scene::BuildMesh(device, commandList);
	Scene::BuildMaterial();
	Scene::BuildAniamtionController();

	// SimulateGlobalTessellationAndWriteFile("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", "Resources/Binarys/Terrain/TerrainBaked.bin");
	tCollider.LoadFromFile("Resources/Binarys/Terrain/TerrainBaked.bin");

	mParticleManager->SetTerrain(device, commandList, tCollider.GetHeader(), tCollider.GetData());

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mMaterialManager->GetMaterial("SkyBoxMaterial");

	mSkyFog.mShader = mShaderMap["SkyFogShader"].get();
	mSkyFog.mMesh = mMeshMap["SkyFog"].get();
	mSkyFog.mMaterial = mMaterialManager->GetMaterial("SkyFogMaterial");


	 //Scene::BakeEnvironment ("Resources/Binarys/Terrain/Environment.bin");
	 Scene::BuildEnvironment("Resources/Binarys/Terrain/env1.bin");





	{
		auto& object = mGameObjects.emplace_back(); 
		object.mShader = mShaderMap["TerrainShader"].get();
		object.mMesh = mMeshMap["Terrain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("TerrainMaterial");
		object.SetActiveState(true);

		object.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };
		object.GetTransform().Scaling(1.f, 1.f, 1.f);
	}

	//std::ofstream file{ "Resources/Binarys/Terrain/env_bb.bin", std::ios::binary };
	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Tree1);
	//	auto& bb = mColliderMap["Pine3_Stem"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Tree2);
	//	auto& bb = mColliderMap["Pine2"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Tree3);
	//	auto& bb = mColliderMap["Pine4"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Rock1);
	//	auto& bb = mColliderMap["Rock_1"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Rock2);
	//	auto& bb = mColliderMap["Rock_2"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Rock3);
	//	auto& bb = mColliderMap["Rock_3"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Rock4);
	//	auto& bb = mColliderMap["Rock_4"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::LargeRock1);
	//	auto& bb = mColliderMap["LargeRock1"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::LargeRock2);
	//	auto& bb = mColliderMap["LargeRock2"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Mountain1);
	//	auto& bb = mColliderMap["Mountain"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Mountain2);
	//	auto& bb = mColliderMap["Mountain1"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}
	//
	//{
	//	auto type = static_cast<UINT>(EnvironmentType::TimberHouse);
	//	auto& bb = mColliderMap["TimberHouse"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::StoneHouse);
	//	auto& bb = mColliderMap["StoneHouse"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::LogHouse);
	//	auto& bb = mColliderMap["LogHouse"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::WindMill);
	//	auto& bb = mColliderMap["WindMill"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}

	//{
	//	auto type = static_cast<UINT>(EnvironmentType::Well);
	//	auto& bb = mColliderMap["Well"];
	//	auto offset = SimpleMath::Vector3{ 0.f, bb.GetOriginCenter().y, 0.f };

	//	file.write(reinterpret_cast<const char*>(&type), sizeof(UINT));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//	file.write(reinterpret_cast<const char*>(&offset), sizeof(SimpleMath::Vector3));
	//}


	{
		mEquipments["Sword"] = EquipmentObject{}; 
		mEquipments["Sword"].mMesh = mMeshMap["Sword"].get();
		mEquipments["Sword"].mShader = mShaderMap["StandardShader"].get(); 
		mEquipments["Sword"].mMaterial = mMaterialManager->GetMaterial("SwordMaterial");
		mEquipments["Sword"].mCollider = mColliderMap["Sword"];
		mEquipments["Sword"].mEquipJointIndex = 58;
		mEquipments["Sword"].SetActiveState(true);

	}



	for (auto& environment : mEnvironmentObjects) {
		environment.UpdateShaderVariables(); 
	}

	mGameObjects.resize(MeshRenderManager::MAX_INSTANCE_COUNT<size_t>, GameObject{});



	mCamera = Camera(mainCameraBufferLocation);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });

	ParticleVertex v{};

	v.position = DirectX::XMFLOAT3(10.f, 10.f, 10.f);
	v.halfheight = 0.5f;
	v.halfWidth = 0.5f;
	v.material = 0;

	v.spritable = false;


	v.direction = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
	v.velocity = 0.f;
	v.totalLifeTime = 0.1f;
	v.lifeTime = 0.1f;

	v.type = ParticleType_emit;
	v.emitType = ParticleType_ember;
	v.remainEmit = 100000;
	v.emitIndex = 0;


	test = mParticleManager->CreateEmitParticle(commandList, v);

	v.position = DirectX::XMFLOAT3(10.f, 20.f, 10.f);
	test1 = mParticleManager->CreateEmitParticle(commandList, v);

	v.position = DirectX::XMFLOAT3(10.f, 30.f, 10.f);
	test2 = mParticleManager->CreateEmitParticle(commandList, v);


	Scene::BuildPacketProcessor();
}

Scene::~Scene() {
	gClientCore->End(); 
}

void Scene::ProcessNetwork() {
	auto packetHandler = gClientCore->GetPacketHandler(); 
	decltype(auto) buffer = packetHandler->GetBuffer(); 

	mPacketProcessor.ProcessPackets(buffer);
}

void Scene::Update() {
	if (mMyPlayer != nullptr) {
		auto& pos = mMyPlayer->GetTransform().GetPosition();
		pos.y = tCollider.GetHeight(pos.x, pos.z);
	}

	if (mMyPlayer != nullptr) {
		mNetworkInfoText->GetText() = std::format(L"Position : {} {} {}", mCamera.GetTransform().GetPosition().x, mCamera.GetTransform().GetPosition().y, mCamera.GetTransform().GetPosition().z);


		// test.Get()->position = mMyPlayer->GetTransform().GetPosition(); 
		test.Get()->position = mGameObjects[3].GetTransform().GetPosition();
		test1.Get()->position = mGameObjects[4].GetTransform().GetPosition();
		test2.Get()->position = mGameObjects[5].GetTransform().GetPosition();


	}


 
	if (mCameraMode) {
		mCameraMode->Update();
	}
	mCamera.UpdateBuffer(); 

	static BoneTransformBuffer boneTransformBuffer{};

	for (auto& gameObject : mGameObjects) {


		if (gameObject) {
			if (gameObject.mAnimated) {
				gameObject.UpdateShaderVariables(boneTransformBuffer); 
				auto [mesh, shader, modelContext] = gameObject.GetRenderData();
				mMeshRenderManager->AppendBonedMeshContext(shader, mesh, modelContext, boneTransformBuffer);
			}
			else {
				gameObject.UpdateShaderVariables();
				auto [mesh, shader, modelContext] = gameObject.GetRenderData();
				mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
			}
		}
	}


	for (auto& object : mEnvironmentObjects) {
		
		if (object.mCollider.GetActiveState()) {
			if (!mCamera.FrustumCulling(object.mCollider)) {
				continue;
			}
		}
		
		auto [mesh, shader, modelContext] = object.GetRenderData();
		mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
	}


	for (auto& player : mPlayers) {
		if (player.GetActiveState()) {
			player.Update(mMeshRenderManager);
		}
	}





	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition();
	mSkyBox.UpdateShaderVariables();
	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mMeshRenderManager->AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);
}

void Scene::SendNetwork() {
	auto id = gClientCore->GetSessionId();
	PacketCS::PacketKeyInput packetInput{ sizeof(PacketCS::PacketKeyInput), PacketType::PACKET_KEYINPUT, id};

	auto& keyTracker = Input.GetKeyboardTracker();

	for (const auto& key : std::views::iota(static_cast<uint8_t>(0), static_cast<uint8_t>(255))) {
		if (keyTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key)) or keyTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {
			if (keyTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key))) {
				packetInput.key = key;
				packetInput.down = true;
			}
			else if (keyTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {
				packetInput.key = key;
				packetInput.down = false;
			}
			gClientCore->Send(&packetInput); 
		}
	}

	PacketCS::PacketCamera packetCamera{ sizeof(PacketCS::PacketCamera), PacketType::PACKET_CAMERA, id };
	packetCamera.look = mCamera.GetTransform().GetForward();
	packetCamera.position = mCamera.GetTransform().GetPosition();

	gClientCore->Send(&packetCamera);
}

void Scene::BuildPacketProcessor() {
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_PROTOCOL_VERSION, [this](PacketHeader* header) { ProcessPacketProtocolVersion(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_NOTIFY_ID, [this](PacketHeader* header) { ProcessNotifyId(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT, [this](PacketHeader* header) { ProcessObjectPacket(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT_APPEARED, [this](PacketHeader* header) { ProcessObjectAppeared(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT_DISAPPEARED, [this](PacketHeader* header) { ProcessObjectDisappeared(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT_DEAD, [this](PacketHeader* header) { ProcessObjectDead(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_RESTORE_HEALTH, [this](PacketHeader* header) { ProcessRestoreHP(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_USE_ITEM, [this](PacketHeader* header) { ProcessUseItem(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_PLAYER_EXIT, [this](PacketHeader* header) { ProcessPlayerExit(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_ATTACKED, [this](PacketHeader* header) { ProcessObjectAttacked(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_ACQUIRED_ITEM, [this](PacketHeader* header) { ProcessAcquiredItem(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_ANIM_STATE, [this](PacketHeader* header) { ProcessPacketAnimation(header); });
}

void Scene::BuildSendKeyList() {
	mSendKeyList.emplace_back(DirectX::Keyboard::Keys::W);
	mSendKeyList.emplace_back(DirectX::Keyboard::Keys::A);
	mSendKeyList.emplace_back(DirectX::Keyboard::Keys::S);
	mSendKeyList.emplace_back(DirectX::Keyboard::Keys::D);
}

void Scene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	MeshLoader Loader{};
	MeshData data{}; 


	mMeshMap["Cube"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Cube, 1);

	data = Loader.Load("Resources/Assets/Knight/BaseAnim/BaseAnim.gltf");
	mMeshMap["HumanBaseAnim"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Knight/archer.gltf");
	mMeshMap["T_Pose"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Knight/LongSword/LongSword.gltf");
	mMeshMap["SwordMan"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Mountain/Mountain.gltf");
	mMeshMap["Mountain"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Mountain"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Mountain/Mountain1.glb");
	mMeshMap["Mountain1"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Mountain1"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Mountain/Mountain3.glb");
	mMeshMap["Mountain3"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Mountain3"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Monster/MonsterType1.gltf");
	mMeshMap["MonsterType1"] = std::make_unique<Mesh>(device, commandList, data);
	
	data = Loader.Load("Resources/Assets/Weapon/sword/LongSword.glb");
	mMeshMap["Sword"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Sword"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/CorruptedGem/CorruptedGem.glb");
	mMeshMap["CorruptedGem"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["CorruptedGem"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Demon/Demon.glb");
	mMeshMap["Demon"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Tree/PineTree/PineTree.glb", 0);
	mMeshMap["PineTree_Stem"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["PineTree_Stem"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Tree/PineTree/PineTree.glb", 1);
	mMeshMap["PineTree_Leaves"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Env/Fern.glb");
	mMeshMap["Fern"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Fern"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Tree/pine2/pine2.glb");
	mMeshMap["Pine2"] = std::make_unique<Mesh>(device, commandList, data); 
	mColliderMap["Pine2"] = Collider{ data.position };


	//// 파일에 기록할 크기 
	mColliderMap["Pine2"].SetExtents(0.33f, 10.797011f, 0.33f);
	mColliderMap["Pine2"].SetCenter(0.f, 10.7970114f, 0.f);


	data = Loader.Load("Resources/Assets/Tree/pine2/pine3.glb", 0);
	mMeshMap["Pine3_Stem"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Pine3_Stem"] = Collider{ data.position };


	// 파일에 기록할 크기 
	mColliderMap["Pine3_Stem"].SetExtents(0.3f, 7.51479626f, 0.3f);


	data = Loader.Load("Resources/Assets/Tree/pine2/pine3.glb", 1);
	mMeshMap["Pine3_Leaves"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Tree/pine2/pine4.glb");
	mMeshMap["Pine4"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Pine4"] = Collider{ data.position };


	// 파일에 기록할 크기 
	mColliderMap["Pine4"].SetCenter(0.f, 10.7723713f, 0.f);
	mColliderMap["Pine4"].SetExtents(0.35f, 10.7723713f, 0.35f);


	data = Loader.Load("Resources/Assets/Env/Rocks.glb", 0);
	mMeshMap["Rock_1"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Rock_1"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Env/Rocks.glb", 1);
	mMeshMap["Rock_2"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Rock_2"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Env/Rocks.glb", 2);
	mMeshMap["Rock_3"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Rock_3"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Env/Rocks.glb", 3);
	mMeshMap["Rock_4"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Rock_4"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Env/LargeRocks.glb", 0);
	mMeshMap["LargeRock1"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["LargeRock1"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Env/LargeRocks.glb", 1);
	mMeshMap["LargeRock2"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["LargeRock2"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/House/TimberHouse.glb");
	mMeshMap["TimberHouse"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["TimberHouse"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/House/StoneHouse.glb");
	mMeshMap["StoneHouse"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["StoneHouse"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/House/LogHouse.glb", 1);
	mMeshMap["LogHouse"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["LogHouse"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/House/LogHouse.glb", 0);
	mMeshMap["LogHouseDoor"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Env/WindMill.glb", 1);
	mMeshMap["WindMill"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["WindMill"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Env/WindMill.glb", 0);
	mMeshMap["WindMillBlade"] = std::make_unique<Mesh>(device, commandList, data);
	
	data = Loader.Load("Resources/Assets/Env/Well.glb");
	mMeshMap["Well"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Well"] = Collider{ data.position };

	data = tLoader.Load("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", true);
	mMeshMap["Terrain"] = std::make_unique<Mesh>(device, commandList, data);

	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 100.f);
	mMeshMap["SkyFog"] = std::make_unique<Mesh>(device, commandList, 50.f, 40.f, 20);
	mMeshMap["Plane"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Plane, 10);
}

void Scene::BuildMaterial() {
	MaterialConstants mat{};

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Rolling Hills");
	mat.mDiffuseTexture[1] = mTextureManager->GetTexture("dirt_2");
	mMaterialManager->CreateMaterial("TerrainMaterial", mat);


	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("SkyBox_Front_0");
	mat.mDiffuseTexture[1] = mTextureManager->GetTexture("SkyBox_Back_0");
	mat.mDiffuseTexture[2] = mTextureManager->GetTexture("SkyBox_Top_0");
	mat.mDiffuseTexture[3] = mTextureManager->GetTexture("SkyBox_Bottom_0");
	mat.mDiffuseTexture[4] = mTextureManager->GetTexture("SkyBox_Left_0");
	mat.mDiffuseTexture[5] = mTextureManager->GetTexture("SkyBox_Right_0");
	mMaterialManager->CreateMaterial("SkyBoxMaterial", mat);

	mat.mDiffuseColor = { 0.5f, 0.5f, 0.5f, 1.f };
	mMaterialManager->CreateMaterial("SkyFogMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Paladin_diffuse");
	mMaterialManager->CreateMaterial("CubeMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("blue");
	mMaterialManager->CreateMaterial("AreaMat", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Big_02___Default_color");
	mMaterialManager->CreateMaterial("StoneMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Default_OBJ_baseColor");
	mMaterialManager->CreateMaterial("MountainMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("rock_base_color");
	mMaterialManager->CreateMaterial("Mountain1Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("SwordA_v004_Default_AlbedoTransparency");
	mMaterialManager->CreateMaterial("SwordMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Creep_BaseColor");
	mMaterialManager->CreateMaterial("MonsterType1Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("CorrupedGem_BaseColor");
	mMaterialManager->CreateMaterial("CorruptedGemMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("T_BigDemonWarrior_Body_Albedo_Skin_3");
	mMaterialManager->CreateMaterial("DemonMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Tree_0Mat_baseColor");
	mMaterialManager->CreateMaterial("PineTreeStemMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Tree_1Mat_baseColor");
	mMaterialManager->CreateMaterial("PineTreeLeavesMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("ferns");
	mMaterialManager->CreateMaterial("FernMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("pinetree-albedo");
	mMaterialManager->CreateMaterial("Pine2Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("bark01");
	mMaterialManager->CreateMaterial("Pine3StemMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("pinebranch");
	mMaterialManager->CreateMaterial("Pine3LeavesMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("sea-water");
	mMaterialManager->CreateMaterial("WaterMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Small Rock 1 RFS_DefaultMaterial_AlbedoTransparency");
	mMaterialManager->CreateMaterial("Rock_1_Material", mat);
	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Small Rock 2 RFS_DefaultMaterial_AlbedoTransparency");
	mMaterialManager->CreateMaterial("Rock_2_Material", mat);
	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Small Rock 3 RFS_DefaultMaterial_AlbedoTransparency");
	mMaterialManager->CreateMaterial("Rock_3_Material", mat);
	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Small Rock 4 Moss RFS_DefaultMaterial_AlbedoTransparency");
	mMaterialManager->CreateMaterial("Rock_4_Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Large Rock 1 RFS_DefaultMaterial_AlbedoTransparency");
	mMaterialManager->CreateMaterial("LargeRock1_Material", mat);
	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Large Rock 2 RFS_DefaultMaterial_AlbedoTransparency");
	mMaterialManager->CreateMaterial("LargeRock2_Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Timber house_AlbedoTransparency");
	mMaterialManager->CreateMaterial("TimberHouseMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Farmhouse_Albedo");
	mMaterialManager->CreateMaterial("StoneHouseMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Log_House_AlbedoTransparency");
	mMaterialManager->CreateMaterial("LogHouseMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Door_AlbedoTransparency");
	mMaterialManager->CreateMaterial("LogHouseDoorMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("windmill_001_base_COL");
	mMaterialManager->CreateMaterial("WindMillMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("windmill_001_lopatky_COL");
	mMaterialManager->CreateMaterial("WindMillBladeMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("well_albedo");
	mMaterialManager->CreateMaterial("WellMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Mountain3_Diffuse");
	mMaterialManager->CreateMaterial("Mountain3Material", mat);
} 

void Scene::BuildShader(ComPtr<ID3D12Device> device) {
	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);
	mShaderMap["StandardShader"] = std::move(shader);

	shader = std::make_unique<TerrainShader>();
	shader->CreateShader(device);
	mShaderMap["TerrainShader"] = std::move(shader);

	shader = std::make_unique<SkyBoxShader>();
	shader->CreateShader(device);
	mShaderMap["SkyBoxShader"] = std::move(shader);

	shader = std::make_unique<SkyFogShader>();
	shader->CreateShader(device);
	mShaderMap["SkyFogShader"] = std::move(shader);

	shader = std::make_unique<SkinnedShader>();
	shader->CreateShader(device);
	mShaderMap["SkinnedShader"] = std::move(shader);

	shader = std::make_unique<TreeShader>();
	shader->CreateShader(device);
	mShaderMap["TreeShader"] = std::move(shader);
}


void Scene::BuildAniamtionController() {
	Scene::BuildBaseAnimationController();

	Scene::BuildSwordManAnimationController(); 
	Scene::BuildArcherAnimationController();
	Scene::BuildMageAnimationController(); 

	Scene::BuildMonsterType1AnimationController(); 



}

void Scene::BuildEnvironment(const std::filesystem::path& envFile) {

	struct EnvData {
		GameProtocol::EnvironmentType envType;
		SimpleMath::Vector3 position;
		float rotation;
	};

	GameObject stem{};
	GameObject leaves{};

	stem.mShader = mShaderMap["TreeShader"].get();
	stem.mMesh = mMeshMap["Pine3_Stem"].get();
	stem.mMaterial = mMaterialManager->GetMaterial("Pine3StemMaterial");
	stem.SetActiveState(true);
	stem.GetTransform().GetPosition() = { 20.f,tCollider.GetHeight(20.f, 20.f),20.f };
	stem.mCollider = mColliderMap["Pine3_Stem"];

	leaves.mShader = mShaderMap["TreeShader"].get();
	leaves.mMesh = mMeshMap["Pine3_Leaves"].get();
	leaves.mMaterial = mMaterialManager->GetMaterial("Pine3LeavesMaterial");
	leaves.SetActiveState(true);
	leaves.GetTransform().GetPosition() = { 20.f,tCollider.GetHeight(20.f, 20.f),20.f };
	// leaves.mCollider = mColliderMap["Pine3_Stem"];



	GameObject pinetree{};
	pinetree.mShader = mShaderMap["TreeShader"].get();
	pinetree.mMesh = mMeshMap["Pine2"].get();
	pinetree.SetActiveState(true);
	pinetree.mMaterial = mMaterialManager->GetMaterial("Pine2Material");
	pinetree.mCollider = mColliderMap["Pine2"];

	GameObject pinetree2{};
	pinetree2.mShader = mShaderMap["TreeShader"].get();
	pinetree2.mMesh = mMeshMap["Pine4"].get();
	pinetree2.SetActiveState(true);
	pinetree2.mMaterial = mMaterialManager->GetMaterial("Pine2Material");
	pinetree2.mCollider = mColliderMap["Pine4"];

	GameObject rock1{};
	rock1.mShader = mShaderMap["StandardShader"].get();
	rock1.mMesh = mMeshMap["Rock_1"].get();
	rock1.mMaterial = mMaterialManager->GetMaterial("Rock_1_Material");
	rock1.SetActiveState(true);
	rock1.mCollider = mColliderMap["Rock_1"];

	GameObject rock2{};
	rock2.mShader = mShaderMap["StandardShader"].get();
	rock2.mMesh = mMeshMap["Rock_2"].get();
	rock2.mMaterial = mMaterialManager->GetMaterial("Rock_2_Material");
	rock2.SetActiveState(true);
	rock2.mCollider = mColliderMap["Rock_2"];

	GameObject rock3{};
	rock3.mShader = mShaderMap["StandardShader"].get();
	rock3.mMesh = mMeshMap["Rock_3"].get();
	rock3.mMaterial = mMaterialManager->GetMaterial("Rock_3_Material");
	rock3.SetActiveState(true);
	rock3.mCollider = mColliderMap["Rock_3"];

	GameObject rock4{};
	rock4.mShader = mShaderMap["StandardShader"].get();
	rock4.mMesh = mMeshMap["Rock_4"].get();
	rock4.mMaterial = mMaterialManager->GetMaterial("Rock_4_Material");
	rock4.SetActiveState(true);
	rock4.mCollider = mColliderMap["Rock_4"];

	GameObject bigrock1{};
	bigrock1.mShader = mShaderMap["StandardShader"].get();
	bigrock1.mMesh = mMeshMap["LargeRock1"].get();
	bigrock1.mMaterial = mMaterialManager->GetMaterial("LargeRock1_Material");
	bigrock1.SetActiveState(true);
	bigrock1.mCollider = mColliderMap["LargeRock1"];

	GameObject bigrock2{};
	bigrock2.mShader = mShaderMap["StandardShader"].get();
	bigrock2.mMesh = mMeshMap["LargeRock2"].get();
	bigrock2.mMaterial = mMaterialManager->GetMaterial("LargeRock2_Material");
	bigrock2.SetActiveState(true);
	bigrock2.mCollider = mColliderMap["LargeRock2"];

	GameObject fern{};
	fern.mShader = mShaderMap["TreeShader"].get();
	fern.mMesh = mMeshMap["Fern"].get();
	fern.mMaterial = mMaterialManager->GetMaterial("FernMaterial");
	fern.SetActiveState(true);
	//fern.mCollider = mColliderMap["Fern"];

	GameObject baseMountain;
	baseMountain.mShader = mShaderMap["StandardShader"].get();
	baseMountain.mMesh = mMeshMap["Mountain"].get();
	baseMountain.mMaterial = mMaterialManager->GetMaterial("MountainMaterial");
	baseMountain.mCollider = mColliderMap["Mountain"];

	GameObject baseMountain1 = baseMountain.Clone();
	baseMountain1.mMesh = mMeshMap["Mountain1"].get();
	baseMountain1.mMaterial = mMaterialManager->GetMaterial("Mountain1Material");
	baseMountain1.mCollider = mColliderMap["Mountain1"];

	GameObject baseMountain2 = baseMountain.Clone();
	baseMountain2.mMesh = mMeshMap["Mountain3"].get();
	baseMountain2.mMaterial = mMaterialManager->GetMaterial("Mountain3Material");
	baseMountain2.mCollider = mColliderMap["Mountain3"];

	GameObject baseTimberHouse;
	baseTimberHouse.mShader = mShaderMap["StandardShader"].get();
	baseTimberHouse.mMesh = mMeshMap["TimberHouse"].get();
	baseTimberHouse.mMaterial = mMaterialManager->GetMaterial("TimberHouseMaterial");
	baseTimberHouse.mCollider = mColliderMap["TimberHouse"];

	GameObject baseStoneHouse = baseTimberHouse.Clone();
	baseStoneHouse.mMesh = mMeshMap["StoneHouse"].get();
	baseStoneHouse.mMaterial = mMaterialManager->GetMaterial("StoneHouseMaterial");
	baseStoneHouse.mCollider = mColliderMap["StoneHouse"];

	GameObject baseLogHouse = baseTimberHouse.Clone();
	baseLogHouse.mMesh = mMeshMap["LogHouse"].get();
	baseLogHouse.mMaterial = mMaterialManager->GetMaterial("LogHouseMaterial");
	baseLogHouse.mCollider = mColliderMap["LogHouse"];

	GameObject baseLogHouseDoor = baseLogHouse.Clone();
	baseLogHouseDoor.mMesh = mMeshMap["LogHouseDoor"].get();
	baseLogHouseDoor.mMaterial = mMaterialManager->GetMaterial("LogHouseDoorMaterial");
	//baseLogHouseDoor.mCollider = mColliderMap["LogHouse"];

	GameObject baseWindMill;
	baseWindMill.mShader = mShaderMap["StandardShader"].get();
	baseWindMill.mMesh = mMeshMap["WindMill"].get();
	baseWindMill.mMaterial = mMaterialManager->GetMaterial("WindMillMaterial");
	baseWindMill.mCollider = mColliderMap["WindMill"];

	GameObject baseWindMillBlade = baseWindMill.Clone();
	baseWindMillBlade.mShader = mShaderMap["TreeShader"].get();
	baseWindMillBlade.mMesh = mMeshMap["WindMillBlade"].get();
	baseWindMillBlade.mMaterial = mMaterialManager->GetMaterial("WindMillBladeMaterial");
	//baseWindMillBlade.mCollider = mColliderMap["WindMill"];

	GameObject baseWater;
	baseWater.mShader = mShaderMap["StandardShader"].get();
	baseWater.mMesh = mMeshMap["Plane"].get();
	baseWater.mMaterial = mMaterialManager->GetMaterial("WaterMaterial");

	GameObject baseWell = baseTimberHouse.Clone();
	baseWell.mMesh = mMeshMap["Well"].get();
	baseWell.mMaterial = mMaterialManager->GetMaterial("WellMaterial");
	baseWell.mCollider = mColliderMap["Well"];


	std::ifstream ifs(envFile, std::ios::binary);
	if (!ifs) {
		return;
	}

	UINT envCount{};
	ifs.read(reinterpret_cast<char*>(&envCount), sizeof(UINT));

	std::vector<EnvData> envPoses(envCount);
	ifs.read(reinterpret_cast<char*>(envPoses.data()), sizeof(EnvData) * envCount);


	// 이후 나무 객체를 생성하는 부분 (stem, leaves 복제)
	std::vector<GameObject> envObjects{};
	for (auto& envData : envPoses) {
		switch (envData.envType) {
		case GameProtocol::EnvironmentType::Tree1:
		{
			{
				auto& object = envObjects.emplace_back();
				object = stem.Clone();
				object.GetTransform().SetPosition(envData.position);
			}
			{
				auto& object = envObjects.emplace_back();
				object = leaves.Clone();
				object.GetTransform().SetPosition(envData.position);
			}
		}
		break;
		case GameProtocol::EnvironmentType::Tree2:
		{
			auto& object = envObjects.emplace_back();
			object = pinetree.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Tree3:
		{
			auto& object = envObjects.emplace_back();
			object = pinetree2.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Rock1:
		{
			auto& object = envObjects.emplace_back();
			object = rock1.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Rock2:
		{
			auto& object = envObjects.emplace_back();
			object = rock2.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Rock3:
		{
			auto& object = envObjects.emplace_back();
			object = rock3.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Rock4:
		{
			auto& object = envObjects.emplace_back();
			object = rock4.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::LargeRock1:
		{
			auto& object = envObjects.emplace_back();
			object = bigrock1.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::LargeRock2:
		{
			auto& object = envObjects.emplace_back();
			object = bigrock2.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Fern:
		{
			auto& object = envObjects.emplace_back();
			object = fern.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case GameProtocol::EnvironmentType::Mountain1:
		{
			auto& object = envObjects.emplace_back();
			object = baseMountain.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::Mountain2:
		{
			auto& object = envObjects.emplace_back();
			object = baseMountain1.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::TimberHouse:
		{
			auto& object = envObjects.emplace_back();
			object = baseMountain2.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::StoneHouse:
		{
			auto& object = envObjects.emplace_back();
			object = baseStoneHouse.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::LogHouse:
		{
			auto& object = envObjects.emplace_back();
			object = baseLogHouse.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::LogHouseDoor:
		{
			auto& object = envObjects.emplace_back();
			object = baseLogHouseDoor.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::WindMill:
		{
			auto& object = envObjects.emplace_back();
			object = baseWindMill.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::WindMillBlade:
		{
			auto& object = envObjects.emplace_back();
			object = baseWindMillBlade.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		case GameProtocol::EnvironmentType::Well:
		{
			auto& object = envObjects.emplace_back();
			object = baseWell.Clone();
			object.GetTransform().SetPosition(envData.position);
			object.GetTransform().Rotate(0.f, envData.rotation, 0.f);
		}
		break;
		default:
			break;
		}
	}

	std::move(envObjects.begin(), envObjects.end(), std::back_inserter(mEnvironmentObjects));
}
void Scene::BakeEnvironment(const std::filesystem::path& path) {

	const float treeMinDistance = 2.f;         
	const int interiorTreeCount = 1000;          
	std::vector<SimpleMath::Vector3> interiorTreePoses;
	interiorTreePoses.reserve(interiorTreeCount);


	const int borderTreeCount = 300;
	std::vector<SimpleMath::Vector3> borderTreePoses;
	borderTreePoses.reserve(borderTreeCount);

	const float rockMinDistance = 1.f;           
	const int rockCount = 500;
	std::vector<SimpleMath::Vector3> rockPoses;
	rockPoses.reserve(rockCount);

	const float bigRockMinDistance = 5.f;        
	const int bigRockCount = 300;
	std::vector<SimpleMath::Vector3> bigRockPoses;
	bigRockPoses.reserve(bigRockCount);

	const int grassCount = 1000;
	std::vector<SimpleMath::Vector3> grassPoses;
	grassPoses.reserve(grassCount);


	std::default_random_engine dre(std::random_device{}());

	std::uniform_real_distribution<float> xPosInterior(-250.f, 250.f);
	std::uniform_real_distribution<float> zPosInterior(-250.f, 250.f);

	std::uniform_real_distribution<float> xPosBorder(-512.f, 512.f);
	std::uniform_real_distribution<float> zPosBorder(-512.f, 512.f);

	std::uniform_int_distribution<int> interiorTreeTypeDist(0, 2);
	for (int i = 0; i < interiorTreeCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPosInterior(dre);
			pos.z = zPosInterior(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z) - 0.5f;
			validPos = true;
			for (const auto& other : interiorTreePoses) {
				float dx = pos.x - other.x;
				float dz = pos.z - other.z;
				if ((dx * dx + dz * dz) < (treeMinDistance * treeMinDistance)) {
					validPos = false;
					break;
				}
			}
		}
		interiorTreePoses.push_back(pos);
	}


	while (static_cast<int>(borderTreePoses.size()) < borderTreeCount) {
		SimpleMath::Vector3 pos;
		pos.x = xPosBorder(dre);
		pos.z = zPosBorder(dre);

		if (pos.x >= -250.f && pos.x <= 250.f &&
			pos.z >= -250.f && pos.z <= 250.f)
			continue;
		pos.y = tCollider.GetHeight(pos.x, pos.z) - 0.5f;

		if (pos.y < 110.f)
			continue;

		bool validPos = true;
		for (const auto& other : borderTreePoses) {
			float dx = pos.x - other.x;
			float dz = pos.z - other.z;
			if ((dx * dx + dz * dz) < (treeMinDistance * treeMinDistance)) {
				validPos = false;
				break;
			}
		}
		if (!validPos)
			continue;
		borderTreePoses.push_back(pos);
	}


	std::uniform_int_distribution<int> rockTypeDist(3, 6);
	for (int i = 0; i < rockCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPosInterior(dre);
			pos.z = zPosInterior(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z);
			validPos = true;

			for (const auto& other : rockPoses) {
				float dx = pos.x - other.x;
				float dz = pos.z - other.z;
				if ((dx * dx + dz * dz) < (rockMinDistance * rockMinDistance)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;

			const float epsilon = 0.01f;
			for (const auto& tree : interiorTreePoses) {
				float dx = pos.x - tree.x;
				float dz = pos.z - tree.z;
				if ((dx * dx + dz * dz) < (epsilon * epsilon)) {
					validPos = false;
					break;
				}
			}
		}
		rockPoses.push_back(pos);
	}


	std::uniform_int_distribution<int> bigRockTypeDist(7, 8);
	for (int i = 0; i < bigRockCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPosInterior(dre);
			pos.z = zPosInterior(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z);
			validPos = true;

			for (const auto& tree : interiorTreePoses) {
				float dx = pos.x - tree.x;
				float dz = pos.z - tree.z;
				if ((dx * dx + dz * dz) < (bigRockMinDistance * bigRockMinDistance)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;

			for (const auto& rock : rockPoses) {
				float dx = pos.x - rock.x;
				float dz = pos.z - rock.z;
				if ((dx * dx + dz * dz) < (bigRockMinDistance * bigRockMinDistance)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;

			for (const auto& bRock : bigRockPoses) {
				float dx = pos.x - bRock.x;
				float dz = pos.z - bRock.z;
				if ((dx * dx + dz * dz) < (bigRockMinDistance * bigRockMinDistance)) {
					validPos = false;
					break;
				}
			}
		}
		bigRockPoses.push_back(pos);
	}


	for (int i = 0; i < grassCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPosInterior(dre);
			pos.z = zPosInterior(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z);
			validPos = true;
			const float epsilon = 0.01f;
			for (const auto& tree : interiorTreePoses) {
				float dx = pos.x - tree.x;
				float dz = pos.z - tree.z;
				if ((dx * dx + dz * dz) < (epsilon * epsilon)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;
			for (const auto& rock : rockPoses) {
				float dx = pos.x - rock.x;
				float dz = pos.z - rock.z;
				if ((dx * dx + dz * dz) < (epsilon * epsilon)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;
			for (const auto& bRock : bigRockPoses) {
				float dx = pos.x - bRock.x;
				float dz = pos.z - bRock.z;
				if ((dx * dx + dz * dz) < (epsilon * epsilon)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;
			for (const auto& grass : grassPoses) {
				float dx = pos.x - grass.x;
				float dz = pos.z - grass.z;
				if ((dx * dx + dz * dz) < (epsilon * epsilon)) {
					validPos = false;
					break;
				}
			}
		}
		grassPoses.push_back(pos);
	}


	const int region1TreeCount = 230;
	std::vector<SimpleMath::Vector3> region1TreePoses;
	region1TreePoses.reserve(region1TreeCount);
	std::uniform_real_distribution<float> xPosRegion1(250.f, 260.f);
	std::uniform_real_distribution<float> zPosRegion1(60.f, -244.f);
	std::uniform_int_distribution<int> treeTypeDist(1, 2);

	for (int i = 0; i < region1TreeCount; i++) {
		SimpleMath::Vector3 pos;
		pos.x = xPosRegion1(dre);
		pos.z = zPosRegion1(dre);
		pos.y = tCollider.GetHeight(pos.x, pos.z) - 0.5f;
		region1TreePoses.push_back(pos);
	}


	const int region2TreeCount = 120;
	std::vector<SimpleMath::Vector3> region2TreePoses;
	region2TreePoses.reserve(region2TreeCount);
	std::uniform_real_distribution<float> xPosRegion2(55.f, 201.f);
	std::uniform_real_distribution<float> zPosRegion2(-250.f, -260.f);

	for (int i = 0; i < region2TreeCount; i++) {
		SimpleMath::Vector3 pos;
		pos.x = xPosRegion2(dre);
		pos.z = zPosRegion2(dre);
		pos.y = tCollider.GetHeight(pos.x, pos.z) - 0.5f;
		region2TreePoses.push_back(pos);
	}



	std::ofstream ofs(path, std::ios::binary);
	if (!ofs) {
		return;
	}

	UINT totalObjects = interiorTreeCount + borderTreeCount + rockCount + bigRockCount + grassCount + region1TreeCount + region2TreeCount;
	ofs.write(reinterpret_cast<const char*>(&totalObjects), sizeof(totalObjects));


	for (const auto& pos : interiorTreePoses) {
		UINT type = static_cast<UINT>(interiorTreeTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}

	std::uniform_int_distribution<int> borderTreeTypeDist(1, 2);
	for (const auto& pos : borderTreePoses) {
		UINT type = static_cast<UINT>(borderTreeTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}


	for (const auto& pos : rockPoses) {
		UINT type = static_cast<UINT>(rockTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}


	for (const auto& pos : bigRockPoses) {
		UINT type = static_cast<UINT>(bigRockTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}


	const UINT grassType = 9;
	for (const auto& pos : grassPoses) {
		ofs.write(reinterpret_cast<const char*>(&grassType), sizeof(grassType));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}


	for (const auto& pos : region1TreePoses) {
		UINT type = static_cast<UINT>(borderTreeTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}


	for (const auto& pos : region2TreePoses) {
		UINT type = static_cast<UINT>(borderTreeTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}

	ofs.close();

	MessageBox(nullptr, L"Environment Baked", L"Success", MB_OK);
}




void Scene::BuildBaseAnimationController() {
	// Base Anim 
	{
		mAnimationMap["HumanBase"].Load("Resources/Assets/Knight/BaseAnim/BaseAnim.gltf");

		std::vector<const AnimationClip*> clips{ mAnimationMap["HumanBase"].GetClip(0), mAnimationMap["HumanBase"].GetClip(1), mAnimationMap["HumanBase"].GetClip(2), mAnimationMap["HumanBase"].GetClip(4), mAnimationMap["HumanBase"].GetClip(3), mAnimationMap["HumanBase"].GetClip(5) };
		std::vector<UINT> boneMask(69);
		std::iota(boneMask.begin(), boneMask.begin() + 59, 0);

		AnimatorGraph::BoneMaskAnimationState idleState{};
		idleState.maskedClipIndex = 0;
		idleState.nonMaskedClipIndex = 0;
		idleState.name = "Idle";

		{
			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.blendDuration = 0.09;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			idleState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.blendDuration = 0.09;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			idleState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.blendDuration = 0.09;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			idleState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.blendDuration = 0.09;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			idleState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.blendDuration = 0.09;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			idleState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState forwardState{};
		forwardState.maskedClipIndex = 1;
		forwardState.nonMaskedClipIndex = 1;
		forwardState.speed = 0.7;
		forwardState.name = "Forward";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.09;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.blendDuration = 0.09;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.blendDuration = 0.09;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.blendDuration = 0.09;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.blendDuration = 0.09;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState backwardState{};
		backwardState.maskedClipIndex = 2;
		backwardState.nonMaskedClipIndex = 2;
		backwardState.speed = 1.2;
		backwardState.name = "BackWard";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.09;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.blendDuration = 0.09;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.blendDuration = 0.09;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.blendDuration = 0.09;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.blendDuration = 0.09;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState leftState{};
		leftState.maskedClipIndex = 4;
		leftState.nonMaskedClipIndex = 4;
		leftState.speed = 0.7;
		leftState.name = "Left";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.09;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			leftState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.blendDuration = 0.09;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			leftState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.blendDuration = 0.09;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			leftState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.blendDuration = 0.09;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			leftState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.blendDuration = 0.09;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			leftState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState rightState{};
		rightState.maskedClipIndex = 3;
		rightState.nonMaskedClipIndex = 3;
		rightState.speed = 0.7;
		rightState.name = "Right";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.09;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			rightState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.blendDuration = 0.09;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			rightState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.blendDuration = 0.09;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			rightState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.blendDuration = 0.09;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			rightState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.blendDuration = 0.09;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			rightState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState jumpState{};
		jumpState.maskedClipIndex = 5;
		jumpState.nonMaskedClipIndex = 5;
		jumpState.name = "Jump";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.2;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.blendDuration = 0.09;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.blendDuration = 0.09;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.blendDuration = 0.09;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.blendDuration = 0.09;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toRight);

		}


		mBaseAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, { idleState, forwardState, backwardState, leftState, rightState, jumpState });
		mBaseAnimationController.AddParameter("Move", AnimatorGraph::ParameterType::Int);
		mBaseAnimationController.AddParameter("Jump", AnimatorGraph::ParameterType::Trigger);
		mBaseAnimationController.AddParameter("True", AnimatorGraph::ParameterType::Always);

	}
}

void Scene::BuildArcherAnimationController() {
	// Archer 
	{
		mAnimationMap["Archer"].Load("Resources/Assets/Knight/archer.gltf");

		std::vector<const AnimationClip*> clips{ mAnimationMap["Archer"].GetClip(0), mAnimationMap["Archer"].GetClip(1), mAnimationMap["Archer"].GetClip(16), mAnimationMap["Archer"].GetClip(3) };

		std::vector<UINT> boneMask(69);
		std::iota(boneMask.begin(), boneMask.begin() + 59, 0);
		AnimatorGraph::BoneMaskAnimationState idleState{};
		idleState.maskedClipIndex = 0;
		idleState.nonMaskedClipIndex = 0;
		idleState.name = "Idle";

		{
			AnimatorGraph::AnimationTransition idleToRun{};
			idleToRun.targetStateIndex = 1;
			idleToRun.blendDuration = 0.09;
			idleToRun.parameterName = "Move";
			idleToRun.expectedValue = true;
			idleToRun.triggerOnEnd = false;
			idleState.transitions.emplace_back(idleToRun);
		}

		AnimatorGraph::BoneMaskAnimationState runState{};
		runState.maskedClipIndex = 3;
		runState.nonMaskedClipIndex = 3;
		runState.name = "Move";

		{
			AnimatorGraph::AnimationTransition runToIdle{};
			runToIdle.targetStateIndex = 0;
			runToIdle.blendDuration = 0.09;
			runToIdle.parameterName = "Move";
			runToIdle.expectedValue = false;
			runToIdle.triggerOnEnd = false;
			runState.transitions.emplace_back(runToIdle);

			AnimatorGraph::AnimationTransition runToAttack{};
			runToAttack.targetStateIndex = 2;
			runToAttack.blendDuration = 0.09;
			runToAttack.parameterName = "Attack";
			runToAttack.expectedValue = true;
			runToAttack.triggerOnEnd = false;
			runState.transitions.emplace_back(runToAttack);
		}

		AnimatorGraph::BoneMaskAnimationState runAttack{};
		runAttack.maskedClipIndex = 2;
		runAttack.nonMaskedClipIndex = 1;
		runAttack.name = "Attack";

		{
			AnimatorGraph::AnimationTransition attackToRun{};
			attackToRun.targetStateIndex = 1;
			attackToRun.blendDuration = 0.3;
			attackToRun.parameterName = "Move";
			attackToRun.expectedValue = true;
			attackToRun.triggerOnEnd = true;
			runAttack.transitions.emplace_back(attackToRun);

			AnimatorGraph::AnimationTransition attackToIdle{};
			attackToIdle.targetStateIndex = 0;
			attackToIdle.blendDuration = 0.2;
			attackToIdle.parameterName = "Move";
			attackToIdle.expectedValue = false;
			attackToIdle.triggerOnEnd = false;
			runAttack.transitions.emplace_back(attackToIdle);
		}

		std::vector<AnimatorGraph::BoneMaskAnimationState> states{ idleState, runState,  runAttack };

		mArcherAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
		mArcherAnimationController.AddParameter("Move", AnimatorGraph::ParameterType::Bool);
		mArcherAnimationController.AddParameter("Attack", AnimatorGraph::ParameterType::Trigger);
	}
}

void Scene::BuildSwordManAnimationController() {
	// LongSword
	{
		mAnimationMap["LongSword"].Load("Resources/Assets/Knight/LongSword/LongSword.gltf");
		auto& loader = mAnimationMap["LongSword"];

		std::vector<const AnimationClip*> clips{
			loader.GetClip(0), // Idle
			loader.GetClip(1), // Forward Run
			loader.GetClip(2), // Backward Run
			loader.GetClip(4), // Left Run
			loader.GetClip(3), // Right Run
			loader.GetClip(5), // Jump 
			loader.GetClip(6), // Running Jump 
			loader.GetClip(7), // Attacked 1 
			loader.GetClip(8), // Attacked 2 
			loader.GetClip(9), // Attack 1 
			loader.GetClip(10),// Attack 2
			loader.GetClip(11),// Death 
		};

		std::vector<UINT> boneMask(69);
		std::iota(boneMask.begin(), boneMask.begin() + 59, 0);

		AnimatorGraph::BoneMaskAnimationState idleState{};
		idleState.maskedClipIndex = 0;
		idleState.nonMaskedClipIndex = 0;
		idleState.name = "Idle";

		{
			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			idleState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			idleState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			idleState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			idleState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			idleState.transitions.emplace_back(toJump);

			AnimatorGraph::AnimationTransition toAttack{};
			toAttack.targetStateIndex = 7;
			toAttack.parameterName = "Attack";
			toAttack.expectedValue = true;
			toAttack.triggerOnEnd = false;
			idleState.transitions.emplace_back(toAttack);
		}

		AnimatorGraph::BoneMaskAnimationState forwardState{};
		forwardState.maskedClipIndex = 1;
		forwardState.nonMaskedClipIndex = 1;
		forwardState.name = "Forward";
		forwardState.speed = 1.2;

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 6;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			forwardState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState backwardState{};
		backwardState.maskedClipIndex = 2;
		backwardState.nonMaskedClipIndex = 2;
		backwardState.name = "BackWard";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.blendDuration = 0.09;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			backwardState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState leftState{};
		leftState.maskedClipIndex = 4;
		leftState.nonMaskedClipIndex = 4;
		leftState.name = "Left";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			leftState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			leftState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			leftState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = false;
			leftState.transitions.emplace_back(toRight);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			leftState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState rightState{};
		rightState.maskedClipIndex = 3;
		rightState.nonMaskedClipIndex = 3;
		rightState.name = "Right";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = false;
			rightState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = false;
			rightState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = false;
			rightState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = false;
			rightState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toJump{};
			toJump.targetStateIndex = 5;
			toJump.parameterName = "Jump";
			toJump.expectedValue = true;
			toJump.triggerOnEnd = false;
			rightState.transitions.emplace_back(toJump);
		}

		AnimatorGraph::BoneMaskAnimationState jumpState{};
		jumpState.maskedClipIndex = 5;
		jumpState.nonMaskedClipIndex = 5;
		jumpState.name = "Jump";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.2;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = true;
			jumpState.transitions.emplace_back(toRight);

		}

		AnimatorGraph::BoneMaskAnimationState runningJump{};
		runningJump.maskedClipIndex = 6;
		runningJump.nonMaskedClipIndex = 6;
		runningJump.name = "RunningJump";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.2;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = true;
			runningJump.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = true;
			runningJump.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = true;
			runningJump.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = true;
			runningJump.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = true;
			runningJump.transitions.emplace_back(toRight);
		}

		AnimatorGraph::BoneMaskAnimationState attack{};
		attack.maskedClipIndex = 9;
		attack.nonMaskedClipIndex = 9;
		attack.name = "Attack";

		{
			AnimatorGraph::AnimationTransition toIdle{};
			toIdle.targetStateIndex = 0;
			toIdle.blendDuration = 0.2;
			toIdle.parameterName = "Move";
			toIdle.expectedValue = 0;
			toIdle.triggerOnEnd = true;
			attack.transitions.emplace_back(toIdle);

			AnimatorGraph::AnimationTransition toForward{};
			toForward.targetStateIndex = 1;
			toForward.parameterName = "Move";
			toForward.expectedValue = 1;
			toForward.triggerOnEnd = true;
			attack.transitions.emplace_back(toForward);

			AnimatorGraph::AnimationTransition toBackward{};
			toBackward.targetStateIndex = 2;
			toBackward.parameterName = "Move";
			toBackward.expectedValue = 2;
			toBackward.triggerOnEnd = true;
			attack.transitions.emplace_back(toBackward);

			AnimatorGraph::AnimationTransition toLeft{};
			toLeft.targetStateIndex = 3;
			toLeft.parameterName = "Move";
			toLeft.expectedValue = 3;
			toLeft.triggerOnEnd = true;
			attack.transitions.emplace_back(toLeft);

			AnimatorGraph::AnimationTransition toRight{};
			toRight.targetStateIndex = 4;
			toRight.parameterName = "Move";
			toRight.expectedValue = 4;
			toRight.triggerOnEnd = true;
			attack.transitions.emplace_back(toRight);
		}


		mSwordManAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, { idleState, forwardState, backwardState, leftState, rightState, jumpState, runningJump, attack });
		mSwordManAnimationController.AddParameter("Move", AnimatorGraph::ParameterType::Int);
		mSwordManAnimationController.AddParameter("Jump", AnimatorGraph::ParameterType::Trigger);
		mSwordManAnimationController.AddParameter("Attack", AnimatorGraph::ParameterType::Trigger);
		mSwordManAnimationController.AddParameter("True", AnimatorGraph::ParameterType::Always);
	}
}

void Scene::BuildMageAnimationController() {

}

void Scene::BuildMonsterType1AnimationController() {
	mAnimationMap["MonsterType1"].Load("Resources/Assets/Monster/MonsterType1.gltf");
	auto& loader = mAnimationMap["MonsterType1"];

	std::vector<AnimationClip*> clips = {
		loader.GetClip(0), // Bite 
		loader.GetClip(1), // Crouch 
		loader.GetClip(2), // Death
		loader.GetClip(3), // Eat
		loader.GetClip(4), // Idle1
		loader.GetClip(5), // Idle2 
		loader.GetClip(6), // Punch 
		loader.GetClip(7), // Roar 
		loader.GetClip(8), // Sniff 
		loader.GetClip(9), // Walk1 
		loader.GetClip(10) // Walk2 
	};


	AnimatorGraph::AnimationState Idle{}; 
	Idle.clip = clips[4];
	Idle.name = "Idle";
	
	{
		AnimatorGraph::AnimationTransition toWalk{};
		toWalk.targetStateIndex = 1;
		toWalk.blendDuration = 0.09;
		toWalk.parameterName = "Move";
		toWalk.expectedValue = 1;
		toWalk.triggerOnEnd = false;
		Idle.transitions.emplace_back(toWalk);
	};

	AnimatorGraph::AnimationState Walk{};
	Walk.clip = clips[9];
	Walk.name = "Walk";

	{
		AnimatorGraph::AnimationTransition toIdle{};
		toIdle.targetStateIndex = 0;
		toIdle.blendDuration = 0.09;
		toIdle.parameterName = "Move";
		toIdle.expectedValue = 0;
		toIdle.triggerOnEnd = false;
		Walk.transitions.emplace_back(toIdle);
	};

	mMonsterType1AnimationController = AnimatorGraph::AnimationGraphController({ Idle, Walk });

}

void Scene::BuildDemonAnimationController() {
	mAnimationMap["Demon"].Load("Resources/Assets/Demon/Demon.glb");
	auto& loader = mAnimationMap["MonsterType1"];

	std::vector<AnimationClip*> clips = {
		loader.GetClip(0), // Bite 
		loader.GetClip(1), // Crouch 
		loader.GetClip(2), // Death
		loader.GetClip(3), // Eat
		loader.GetClip(4), // Idle1
		loader.GetClip(5), // Idle2 
		loader.GetClip(6), // Punch 
		loader.GetClip(7), // Roar 
		loader.GetClip(8), // Sniff 
		loader.GetClip(9), // Walk1 
		loader.GetClip(10) // Walk2 
	};
}


void Scene::SetInputBaseAnimMode() {
	Input.EraseCallBack(mInputSign);

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, mInputSign, [this]() {

		if (Input.GetKeyboardState().S) {
			mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		}
		else {
			mMyPlayer->GetBoneMaskController().SetInt("Move", 1);
		}

		});

	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::W, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::S, mInputSign, [this]() {

		if (Input.GetKeyboardState().W)
			mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		else {
			mMyPlayer->GetBoneMaskController().SetInt("Move", 2);
		}

		});

	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::S, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, mInputSign, [this]() {

		if (Input.GetKeyboardState().D)
			mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		else {
			mMyPlayer->GetBoneMaskController().SetInt("Move", 3);
		}

		});

	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::A, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, mInputSign, [this]() {

		if (Input.GetKeyboardState().A)
			mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		else {
			mMyPlayer->GetBoneMaskController().SetInt("Move", 4);
		}
	
		});

	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::D, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetInt("Move", 0);
		});

	Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::Space, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetTrigger("Jump");
		});

	Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::P, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetTrigger("Attack");
		});

}

void Scene::SetInputArcherMode() {

	//Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, mInputSign, [this]() {
	//	mMyPlayer->GetBoneMaskController().SetBool("Move", true);
	//	});

	//Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::W, mInputSign, [this]() {
	//	mMyPlayer->GetBoneMaskController().SetBool("Move", false);
	//	});


	//Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, mInputSign, [this]() {
	//	});

	//Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, mInputSign, [this]() {
	//	});
	//

	//Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F, mInputSign, [this]() {
	//	mMyPlayer->GetBoneMaskController().SetTrigger("Attack");
	//	});
}

void Scene::SetInputSwordManMode() {

}

void Scene::SetInputMageMode() {

}






