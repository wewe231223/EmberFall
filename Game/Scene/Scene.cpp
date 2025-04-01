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

				mMyPlayer->SetWeapon(mWeapons[0]);
				mMyPlayer->SetMyPlayer();
			
				// mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
				mCameraMode = std::make_unique<TPPCameraMode>(&mCamera, mMyPlayer->GetTransform(), SimpleMath::Vector3{ 0.f,2.5f,5.f });
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
	Scene::BuildPacketProcessor();
	Scene::BuildAniamtionController();

	// SimulateGlobalTessellationAndWriteFile("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", "Resources/Binarys/Terrain/TerrainBaked.bin");
	tCollider.LoadFromFile("Resources/Binarys/Terrain/TerrainBaked.bin");

	mParticleManager->SetTerrain(device, commandList, tCollider.GetHeader(), tCollider.GetData());

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mMaterialManager->GetMaterial("SkyBoxMaterial");

	mGameObjects.resize(MeshRenderManager::MAX_INSTANCE_COUNT<size_t>, GameObject{});

	{
		auto& object = mGameObjects[0];
		object.mShader = mShaderMap["TerrainShader"].get();
		object.mMesh = mMeshMap["Terrain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("TerrainMaterial");
		object.SetActiveState(true);

		object.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };
		object.GetTransform().Scaling(1.f, 1.f, 1.f);
	}


	{
		auto& object = mGameObjects[1];
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["Mountain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("MountainMaterial");
		object.mCollider = mColliderMap["Mountain"];
		object.SetActiveState(true);

		object.GetTransform().GetPosition() = { 370.f,tCollider.GetHeight(370.f, 300.f) - 10.f ,300.f };
		object.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(-35.f), 0.f);
	}

	{
		auto& object = mGameObjects[2];
		object.mShader = mShaderMap["SkinnedShader"].get();
		object.mMesh = mMeshMap["MonsterType1"].get();
		object.mMaterial = mMaterialManager->GetMaterial("MonsterType1Material");
		object.mGraphController = mMonsterType1AnimationController;
		object.mAnimated = true;
		object.SetActiveState(true);

		object.GetTransform().GetPosition() = { 0.f, tCollider.GetHeight(0.f, 0.f), 0.f };

	}


	{
		mWeapons[0].mShader = mShaderMap["StandardShader"].get();
		mWeapons[0].mMesh = mMeshMap["Sword"].get();
		mWeapons[0].mMaterial = mMaterialManager->GetMaterial("SwordMaterial");
		mWeapons[0].mCollider = mColliderMap["Sword"];
		mWeapons[0].SetActiveState(true);
	}




	mCamera = Camera(mainCameraBufferLocation);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });

	//mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
	//mCameraMode->Enter(); 




	ParticleVertex v{};

	v.position = DirectX::XMFLOAT3(10.f, 10.f, 10.f);
	v.halfheight = 0.1f;
	v.halfWidth = 0.1f;
	v.material = 0;

	v.spritable = false;


	v.direction = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
	v.velocity = 0.f;
	v.totalLifeTime = 0.01f;
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
	//if (mMyPlayer != nullptr) {
	//	auto& pos = mMyPlayer->GetTransform().GetPosition();
	//	pos.y = tCollider.GetHeight(pos.x, pos.z);
	//}

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
 
	for (auto& player : mPlayers) {
		if (player.GetActiveState()) {
			player.Update(mMeshRenderManager);
		}
	}

	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition();
	mSkyBox.UpdateShaderVariables();
	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mMeshRenderManager->AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);

	mCamera.UpdateBuffer(); 

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

	data = Loader.Load("Resources/Assets/Tree/stem/Ash1.gltf");
	mMeshMap["Tree_Stem"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Tree/leaves1/Ash1.gltf");
	mMeshMap["Tree_Leaves"] = std::make_unique<Mesh>(device, commandList, data);

	mMeshMap["Cube"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Cube, 1);

	data = Loader.Load("Resources/Assets/Knight/BaseAnim/BaseAnim.gltf");
	mMeshMap["HumanBaseAnim"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Knight/archer.gltf");
	mMeshMap["T_Pose"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Knight/LongSword/LongSword.gltf");
	mMeshMap["SwordMan"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone1.gltf");
	mMeshMap["Stone1"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone2.gltf");
	mMeshMap["Stone2"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone3.gltf");
	mMeshMap["Stone3"] = std::make_unique<Mesh>(device, commandList, data);

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



	data = tLoader.Load("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", true);
	mMeshMap["Terrain"] = std::make_unique<Mesh>(device, commandList, data);
	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 100.f);

}

void Scene::BuildMaterial() {
	MaterialConstants mat{};

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Ash_BarkM");
	mMaterialManager->CreateMaterial("TreeMaterial", mat);
	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Leaf_2");
	mMaterialManager->CreateMaterial("TreeLeavesMaterial", mat);

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
}


void Scene::BuildAniamtionController() {
	Scene::BuildBaseAnimationController();

	Scene::BuildSwordManAnimationController(); 
	Scene::BuildArcherAnimationController();
	Scene::BuildMageAnimationController(); 

	Scene::BuildMonsterType1AnimationController(); 

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






