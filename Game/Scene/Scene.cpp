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


	 Scene::BuildEnvironment("Resources/Binarys/Terrain/Environment.bin");
	 //std::thread bakeThread{ [this]() {BakeEnvironment("Resources/Binarys/Terrain/Environment.bin");  } };
	 //bakeThread.detach(); 




	{
		auto& object = mGameObjects.emplace_back(); 
		object.mShader = mShaderMap["TerrainShader"].get();
		object.mMesh = mMeshMap["Terrain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("TerrainMaterial");
		object.SetActiveState(true);

		object.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };
		object.GetTransform().Scaling(1.f, 1.f, 1.f);
	}


	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["Mountain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("MountainMaterial");
		object.mCollider = mColliderMap["Mountain"];
		object.SetActiveState(true);

		object.GetTransform().GetPosition() = { 370.f,tCollider.GetHeight(370.f, 300.f) - 10.f ,300.f };
		object.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(-35.f), 0.f);
	}


	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["Cube"].get();
		object.mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
		object.SetActiveState(true);
		object.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };
		object.GetTransform().Scaling(500.f, 500.f, 500.f);
	}

	{
		mEquipments["Sword"] = EquipmentObject{}; 
		mEquipments["Sword"].mMesh = mMeshMap["Sword"].get();
		mEquipments["Sword"].mShader = mShaderMap["StandardShader"].get(); 
		mEquipments["Sword"].mMaterial = mMaterialManager->GetMaterial("SwordMaterial");
		mEquipments["Sword"].mCollider = mColliderMap["Sword"];
		mEquipments["Sword"].mEquipJointIndex = 58;
		mEquipments["Sword"].SetActiveState(true);

	}



	mGameObjects.resize(MeshRenderManager::MAX_INSTANCE_COUNT<size_t>, GameObject{});



	mCamera = Camera(mainCameraBufferLocation);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });

	//mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
	//mCameraMode->Enter(); 




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
		mNetworkInfoText->GetText() = std::format(L"Position : {} {} {}", mMyPlayer->GetTransform().GetPosition().x, mMyPlayer->GetTransform().GetPosition().y, mMyPlayer->GetTransform().GetPosition().z);


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

				if (gameObject.mCollider.GetActiveState()) {
					if (!mCamera.FrustumCulling(gameObject.mCollider)) {
						continue;
					}
				}

				auto [mesh, shader, modelContext] = gameObject.GetRenderData();
				mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
			}
		}



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

	data = Loader.Load("Resources/Assets/Tree/pine2/pine2.glb");
	mMeshMap["Pine2"] = std::make_unique<Mesh>(device, commandList, data); 
	mColliderMap["Pine2"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Tree/pine2/pine3.glb", 0);
	mMeshMap["Pine3_Stem"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Pine3_Stem"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Tree/pine2/pine3.glb", 1);
	mMeshMap["Pine3_Leaves"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Tree/pine2/pine4.glb");
	mMeshMap["Pine4"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Pine4"] = Collider{ data.position };

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

	data = tLoader.Load("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", true);
	mMeshMap["Terrain"] = std::make_unique<Mesh>(device, commandList, data);
	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 100.f);

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


	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Paladin_diffuse");
	mMaterialManager->CreateMaterial("CubeMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("blue");
	mMaterialManager->CreateMaterial("AreaMat", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Big_02___Default_color");
	mMaterialManager->CreateMaterial("StoneMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Default_OBJ_baseColor");
	mMaterialManager->CreateMaterial("MountainMaterial", mat);

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
		UINT envType;
		SimpleMath::Vector3 position; 
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
	leaves.mCollider = mColliderMap["Pine3_Stem"];



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
		switch (envData.envType){ 
		case 0:
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
		case 1:
		{
			auto& object = envObjects.emplace_back();
			object = pinetree.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case 2:
		{
			auto& object = envObjects.emplace_back();
			object = pinetree2.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case 3:
		{
			auto& object = envObjects.emplace_back();
			object = rock1.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
			break;
		case 4:
		{
			auto& object = envObjects.emplace_back();
			object = rock2.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case 5:
		{
			auto& object = envObjects.emplace_back();
			object = rock3.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case 6:
		{
			auto& object = envObjects.emplace_back();
			object = rock4.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		case 7:
		{
			auto& object = envObjects.emplace_back();
			object = bigrock1.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case 8:
		{
			auto& object = envObjects.emplace_back();
			object = bigrock2.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;
		case 9:
		{
			auto& object = envObjects.emplace_back();
			object = fern.Clone();
			object.GetTransform().SetPosition(envData.position);
		}
		break;

		break;
		default:
			break;
		}
	}

	std::move(envObjects.begin(), envObjects.end(), std::back_inserter(mGameObjects));

}

void Scene::BakeEnvironment(const std::filesystem::path& path) {
	// [1] 나무 레이어 설정
	const float treeMinDistance = 2.f;   // 나무 사이 최소 간격
	const int treeCount = 2000;            // 생성할 나무 개수
	std::vector<SimpleMath::Vector3> treePoses;
	treePoses.reserve(treeCount);

	// [2] 돌 레이어 설정
	const float rockMinDistance = 1.f;     // 돌 사이 최소 간격
	const int rockCount = 2000;            // 생성할 돌 개수
	std::vector<SimpleMath::Vector3> rockPoses;
	rockPoses.reserve(rockCount);

	// [3] 큰 바위 레이어 설정
	const float bigRockMinDistance = 5.f;  // 큰 바위는 모든 오브젝트와 최소 2.f 간격
	const int bigRockCount = 100;          // 생성할 큰 바위 개수
	std::vector<SimpleMath::Vector3> bigRockPoses;
	bigRockPoses.reserve(bigRockCount);

	// [4] 풀 레이어 설정
	const int grassCount = 3000;           // 생성할 풀 개수 (필요에 따라 조정)
	std::vector<SimpleMath::Vector3> grassPoses;
	grassPoses.reserve(grassCount);

	// 난수 생성기 및 좌표 분포 설정 (x, z 좌표 범위 동일)
	std::default_random_engine dre(std::random_device{}());
	std::uniform_real_distribution<float> xPos(-250.f, 250.f);
	std::uniform_real_distribution<float> zPos(-250.f, 250.f);
	std::uniform_int_distribution<int> treeTypeDist(0, 2);   // 나무 타입: 0, 1, 2
	std::uniform_int_distribution<int> rockTypeDist(3, 6);     // 돌 타입: 3, 4, 5, 6
	std::uniform_int_distribution<int> bigRockTypeDist(7, 8);  // 큰 바위 타입: 7 또는 8
	const UINT grassType = 9;                                // 풀 타입: 9

	// [A] 나무 배치 (높이 조건: 40 미만, 최소 거리 조건)
	for (int i = 0; i < treeCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPos(dre);
			pos.z = zPos(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z) - 0.5f;

			validPos = true;
			// 기존 나무들과의 최소 거리 검사 (x,z 평면)
			for (const auto& other : treePoses) {
				float dx = pos.x - other.x;
				float dz = pos.z - other.z;
				if ((dx * dx + dz * dz) < (treeMinDistance * treeMinDistance)) {
					validPos = false;
					break;
				}
			}
		}
		treePoses.push_back(pos);
	}

	// [B] 돌 배치 (자신들 사이 최소 거리 조건 + 나무와 겹침 방지)
	for (int i = 0; i < rockCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPos(dre);
			pos.z = zPos(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z);
			validPos = true;
			// 돌들 사이 최소 거리 검사
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
			// 나무와 겹치지 않도록 (아주 작은 허용 오차)
			const float epsilon = 0.01f;
			for (const auto& tree : treePoses) {
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

	// [C] 큰 바위 배치 (모든 기존 오브젝트와 최소 2.f 간격)
	for (int i = 0; i < bigRockCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPos(dre);
			pos.z = zPos(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z);
			validPos = true;
			// 나무와의 최소 거리 검사
			for (const auto& tree : treePoses) {
				float dx = pos.x - tree.x;
				float dz = pos.z - tree.z;
				if ((dx * dx + dz * dz) < (bigRockMinDistance * bigRockMinDistance)) {
					validPos = false;
					break;
				}
			}
			if (!validPos) continue;
			// 돌과의 최소 거리 검사
			for (const auto& rock : rockPoses) {
				float dx = pos.x - rock.x;
				float dz = pos.z - rock.z;
				if ((dx * dx + dz * dz) < (bigRockMinDistance * bigRockMinDistance)) {
					validPos = false;
					break;
				}
			}
			if (!validPos) continue;
			// 이미 배치된 큰 바위와의 최소 거리 검사
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

	// [D] 풀 배치 (큰 바위와는 최소 2.f, 나머지 오브젝트(나무, 돌, 이미 배치된 풀)와는 정확히 겹치지 않도록)
	for (int i = 0; i < grassCount; i++) {
		SimpleMath::Vector3 pos;
		bool validPos = false;
		while (!validPos) {
			pos.x = xPos(dre);
			pos.z = zPos(dre);
			pos.y = tCollider.GetHeight(pos.x, pos.z);
			validPos = true;
			const float epsilon = 0.01f;  // 아주 작은 허용 오차

			// 나무와의 위치 검사 (정확히 겹치지 않도록)
			for (const auto& tree : treePoses) {
				float dx = pos.x - tree.x;
				float dz = pos.z - tree.z;
				if ((dx * dx + dz * dz) < (epsilon * epsilon)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;
			// 돌과의 위치 검사 (정확히 겹치지 않도록)
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
			// 큰 바위와의 최소 거리 검사 (2.f)
			for (const auto& bRock : bigRockPoses) {
				float dx = pos.x - bRock.x;
				float dz = pos.z - bRock.z;
				if ((dx * dx + dz * dz) < (2.f * 2.f)) {
					validPos = false;
					break;
				}
			}
			if (!validPos)
				continue;
			// 이미 배치된 풀과의 위치 검사 (정확히 겹치지 않도록)
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

	// [E] 파일에 기록 (파일 맨 앞에 총 오브젝트 개수 기록)
	std::ofstream ofs(path, std::ios::binary);
	if (!ofs) {
		// 파일 열기 실패 처리
		return;
	}
	UINT totalObjects = treeCount + rockCount + bigRockCount + grassCount;
	ofs.write(reinterpret_cast<const char*>(&totalObjects), sizeof(totalObjects));

	// 나무 레이어 기록 (타입: 0 ~ 2)
	for (const auto& pos : treePoses) {
		UINT type = static_cast<UINT>(treeTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}

	// 돌 레이어 기록 (타입: 3 ~ 6)
	for (const auto& pos : rockPoses) {
		UINT type = static_cast<UINT>(rockTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}

	// 큰 바위 레이어 기록 (타입: 7 또는 8)
	for (const auto& pos : bigRockPoses) {
		UINT type = static_cast<UINT>(bigRockTypeDist(dre));
		ofs.write(reinterpret_cast<const char*>(&type), sizeof(type));
		ofs.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
		ofs.write(reinterpret_cast<const char*>(&pos.y), sizeof(pos.y));
		ofs.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));
	}

	// 풀 레이어 기록 (타입 고정: 9)
	for (const auto& pos : grassPoses) {
		ofs.write(reinterpret_cast<const char*>(&grassType), sizeof(grassType));
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






