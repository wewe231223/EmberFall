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
void Scene::ProcessPacketProtocolVersion(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ProtocolVersionSC>(buffer);
	if (PROTOCOL_VERSION_MAJOR != data->major() or
		PROTOCOL_VERSION_MINOR != data->minor()) {
		gClientCore->CloseSession();
		MessageBox(nullptr, L"ERROR!!!!!\nProtocolVersion Mismatching", L"", MB_OK | MB_ICONERROR);
		::exit(0);
	}
}

void Scene::ProcessNotifyId(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::NotifyIdSC>(buffer);
	gClientCore->InitSessionId(data->playerId());
}

void Scene::ProcessPlayerExit(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::PlayerExitSC>(buffer);
	if (mPlayerIndexmap.contains(data->playerId())) {
		mPlayerIndexmap[data->playerId()]->SetActiveState(false);
	}
}

void Scene::ProcessLatency(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::PacketLatencySC>(buffer);

}

void Scene::ProcessObjectAppeared(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectAppearedSC>(buffer);

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
		
		
	// 플레이어 등장 
	if (data->objectId() < OBJECT_ID_START) {
		// 내 플레이어 등장 
		if (data->objectId() == gClientCore->GetSessionId()) {
			// 플레이어 인스턴스가 없다면 
			if (not mPlayerIndexmap.contains(data->objectId())) {
		
				auto nextLoc = FindNextPlayerLoc();
		
				if (nextLoc == mPlayers.end()) {
					Crash("There is no more space for My Player!!");
				}
		
				*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mMaterialManager->GetMaterial("CubeMaterial"), mArcherAnimationController);
				mPlayerIndexmap[data->objectId()] = &(*nextLoc);
				mMyPlayer = &(*nextLoc);
		
				mMyPlayer->AddEquipment(mEquipments["Sword"].Clone());
				mMyPlayer->SetMyPlayer();

				mMyPlayer->GetBoneMaskController().Transition(static_cast<size_t>(data->animation()));

					
				//mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
				mCameraMode = std::make_unique<TPPCameraMode>(&mCamera, mMyPlayer->GetTransform(), SimpleMath::Vector3{ 0.f, 1.8f, 3.f });
				mCameraMode->Enter();
			}
			else {
				if (mPlayerIndexmap[data->objectId()] != nullptr) {
					mPlayerIndexmap[data->objectId()]->SetActiveState(true);
				}
			}
		}
		// 다른 플레이어 등장 
		else {
			// 그 플레이어 인스턴스가 없다면  
			if (not mPlayerIndexmap.contains(data->objectId())) {
				auto nextLoc = FindNextPlayerLoc();
				if (nextLoc == mPlayers.end()) { 
					Crash("There is no more space for Other Player!!"); 
				}
		
				*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mMaterialManager->GetMaterial("CubeMaterial"), mSwordManAnimationController);
				mPlayerIndexmap[data->objectId()] = &(*nextLoc);
				mPlayerIndexmap[data->objectId()]->GetBoneMaskController().Transition(static_cast<size_t>(data->animation()));
			}
			else {
				if (mPlayerIndexmap[data->objectId()] != nullptr) {
					mPlayerIndexmap[data->objectId()]->SetActiveState(true);
				}
			}
		}
	}
	// 이외 오브젝트 등장 
	else {
		if (not mGameObjectMap.contains(data->objectId())) {
			auto nextLoc = FindNextObjectLoc();
			if (nextLoc == mGameObjects.end()) {
				Crash("There is no more space for Other Object!!");
			}
			switch (data->entity()) {
				case Packets::EntityType_MONSTER:
				{
					*nextLoc = GameObject{};
					mGameObjectMap[data->objectId()] = &(*nextLoc);
		
					nextLoc->mShader = mShaderMap["SkinnedShader"].get();
					nextLoc->mMesh = mMeshMap["MonsterType1"].get();
					nextLoc->mMaterial = mMaterialManager->GetMaterial("MonsterType1Material");
					nextLoc->mGraphController = mMonsterType1AnimationController;
					nextLoc->mAnimated = true;
					nextLoc->SetActiveState(true);
		

					nextLoc->GetTransform().SetPosition(FbsPacketFactory::GetVector3(data->pos()));
					nextLoc->mGraphController.Transition(static_cast<size_t>(data->animation()));
				}
					break;
				case Packets::EntityType_CORRUPTED_GEM:
				{
					*nextLoc = GameObject{};
					mGameObjectMap[data->objectId()] = &(*nextLoc);
					nextLoc->mShader = mShaderMap["StandardShader"].get();
					nextLoc->mMesh = mMeshMap["CorruptedGem"].get();
					nextLoc->mMaterial = mMaterialManager->GetMaterial("CorruptedGemMaterial");
					nextLoc->SetActiveState(true);
		

					nextLoc->GetTransform().SetPosition(FbsPacketFactory::GetVector3(data->pos()));
				}
					break;
				default:
				{
					*nextLoc = GameObject{};
					mGameObjectMap[data->objectId()] = &(*nextLoc);
					nextLoc->mShader = mShaderMap["StandardShader"].get();
					nextLoc->mMesh = mMeshMap["Cube"].get();
					nextLoc->mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
					nextLoc->SetActiveState(true);

					nextLoc->GetTransform().Scaling(0.3f, 0.3f, 0.3f);
					nextLoc->GetTransform().SetPosition(FbsPacketFactory::GetVector3(data->pos()));
				}
					break;
			}
		}
		
	}
}

void Scene::ProcessObjectDisappeared(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectDisappearedSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {
			mPlayerIndexmap[data->objectId()]->SetActiveState(false);
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->SetActiveState(false);
		}
	}
}

void Scene::ProcessObjectRemoved(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectRemovedSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {
			mPlayerIndexmap[data->objectId()]->SetActiveState(false);
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->SetActiveState(false);
		}
	}
}

void Scene::ProcessObjectMove(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectMoveSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {
			mPlayerIndexmap[data->objectId()]->GetTransform().GetPosition() = FbsPacketFactory::GetVector3(data->pos());


			mPlayerIndexmap[data->objectId()]->GetTransform().SetDirection(FbsPacketFactory::GetVector3(data->dir()));
			mPlayerIndexmap[data->objectId()]->GetTransform().SetSpeed(data->speed());

			if (data->objectId() == gClientCore->GetSessionId()) {
				return;
			}

			auto euler = mPlayerIndexmap[data->objectId()]->GetTransform().GetRotation().ToEuler();
			euler.y = data->yaw();
			mPlayerIndexmap[data->objectId()]->GetTransform().GetRotation() = SimpleMath::Quaternion::CreateFromYawPitchRoll(euler.y, euler.x, euler.z);
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->GetTransform().GetPosition() = FbsPacketFactory::GetVector3(data->pos());
			mGameObjectMap[data->objectId()]->GetTransform().SetDirection(FbsPacketFactory::GetVector3(data->dir()));
			mGameObjectMap[data->objectId()]->GetTransform().SetSpeed(data->speed());
			auto euler = mGameObjectMap[data->objectId()]->GetTransform().GetRotation().ToEuler();
			euler.y = data->yaw();
			mGameObjectMap[data->objectId()]->GetTransform().GetRotation() = SimpleMath::Quaternion::CreateFromYawPitchRoll(euler.y, euler.x, euler.z);
		}
	}
}

void Scene::ProcessObjectAttacked(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectAttackedSC>(buffer);
	// HP 깍기 
}

void Scene::ProcessPacketAnimation(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectAnimationChangedSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {

			mPlayerIndexmap[data->objectId()]->GetBoneMaskController().Transition(static_cast<size_t>(data->animation()));
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->GetAnimationController().Transition(static_cast<size_t>(data->animation()));
		}
	}

}

// 보석 상호작용 전용 패킷 처리 
void Scene::ProcessGemInteraction(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::GemInteractSC>(buffer);
}

void Scene::ProcessGemCancelInteraction(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::GemInteractionCancelSC>(buffer);
}

void Scene::ProcessGemDestroyed(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::GemDestroyedSC>(buffer);

}

// 아이템 
void Scene::ProcessUseItem(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::UseItemSC>(buffer);

}

void Scene::ProcessAcquiredItem(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::AcquiredItemSC>(buffer);


}

// 원거리
void Scene::ProcessFireProjectile(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::FireProjectileSC>(buffer);

}

void Scene::ProcessProjectileMove(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ProjectileMoveSC>(buffer);
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
		auto& boss = mGameObjects.emplace_back();
		boss.mShader = mShaderMap["SkinnedShader"].get();
		boss.mMesh = mMeshMap["Demon"].get();
		boss.mMaterial = mMaterialManager->GetMaterial("DemonMaterial");
		boss.mGraphController = mDemonAnimationController;
		boss.mAnimated = true; 
		boss.SetActiveState(true);
		boss.GetTransform().GetPosition() = { 3.f, tCollider.GetHeight(3.f, 36.f), 36.f};
	}


	{
		mEquipments["Sword"] = EquipmentObject{}; 
		mEquipments["Sword"].mMesh = mMeshMap["Sword"].get();
		mEquipments["Sword"].mShader = mShaderMap["StandardShader"].get(); 
		mEquipments["Sword"].mMaterial = mMaterialManager->GetMaterial("SwordMaterial");
		mEquipments["Sword"].mCollider = mColliderMap["Sword"];
		mEquipments["Sword"].mEquipJointIndex = 36;
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
}

Scene::~Scene() {
	gClientCore->End(); 
}

void Scene::ProcessNetwork() {
	auto packetHandler = gClientCore->GetPacketHandler(); 
	decltype(auto) buffer = packetHandler->GetBuffer(); 

	Scene::ProcessPackets(reinterpret_cast<const uint8_t*>(buffer.Data()), buffer.Size());
	
}

void Scene::ProcessPackets(const uint8_t* buffer, size_t size) { 
	const uint8_t* iter = buffer; 
	
	while (iter < buffer + size) {
		iter = ProcessPacket(iter);
	}

}

const uint8_t* Scene::ProcessPacket(const uint8_t* buffer) {
	decltype(auto) header = FbsPacketFactory::GetHeaderPtrSC(buffer); 
	
	switch (header->type) {
	case Packets::PacketTypes_PT_PROTOCOL_VERSION_SC:
	{
		Scene::ProcessPacketProtocolVersion(buffer);
	}
	break;
	case Packets::PacketTypes_PT_NOTIFY_ID_SC:
	{
		Scene::ProcessNotifyId(buffer);
	}
	break; 
	case Packets::PacketTypes_PT_OBJECT_REMOVED_SC:
	{
		Scene::ProcessObjectRemoved(buffer);
	}
	break;
	case Packets::PacketTypes_PT_PLAYER_EXIT_SC:
	{
		Scene::ProcessPlayerExit(buffer);
	}
	break;
	case Packets::PacketTypes_PT_LATENCT_SC:
	{
		Scene::ProcessLatency(buffer);
	}
	break; 
	case Packets::PacketTypes_PT_OBJECT_APPEARED_SC:
	{
		Scene::ProcessObjectAppeared(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_DISAPPEARED_SC:
	{
		Scene::ProcessObjectDisappeared(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_MOVE_SC:
	{
		Scene::ProcessObjectMove(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_ATTACKED_SC:
	{
		Scene::ProcessObjectAttacked(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_ANIMATION_CHANGED_SC:
	{
		Scene::ProcessPacketAnimation(buffer);
	}
	break;
	case Packets::PacketTypes_PT_GEM_INTERACT_SC:
	{
		Scene::ProcessGemInteraction(buffer);
	}
	break;
	case Packets::PacketTypes_PT_GEM_CANCEL_INTERACTOIN_SC:
	{
		Scene::ProcessGemCancelInteraction(buffer);
	}
	break;
	case Packets::PacketTypes_PT_GEM_DESTROYED_SC:
	{
		Scene::ProcessGemDestroyed(buffer);
	}
	break;
	case Packets::PacketTypes_PT_USE_ITEM_SC:
	{
		Scene::ProcessUseItem(buffer);
	}
	break;
	case Packets::PacketTypes_PT_ACQUIRED_ITEM_SC:
	{
		Scene::ProcessAcquiredItem(buffer);
	}
	break;
	case Packets::PacketTypes_PT_FIRE_PROJECTILE_SC:
	{
		Scene::ProcessFireProjectile(buffer);
	}
	break;
	case Packets::PacketTypes_PT_PROJECTILE_MOVE_SC:
	{
		Scene::ProcessProjectileMove(buffer);
	}
	break;
	default:
		break;
	}

	return buffer + header->size; 
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
	auto& keyTracker = Input.GetKeyboardTracker();

	for (const auto& key : std::views::iota(static_cast<uint8_t>(0), static_cast<uint8_t>(255))) {
		if (keyTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key)) or keyTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {

			if (keyTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key))) {
				decltype(auto) packet = FbsPacketFactory::PlayerInputCS(id, key, true);
				gClientCore->Send(packet);
			}
			else if (keyTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {
				decltype(auto) packet = FbsPacketFactory::PlayerInputCS(id, key, false);
				gClientCore->Send(packet);
			}

		}
	}

	decltype(auto) packetCamera = FbsPacketFactory::PlayerLookCS(id, mCamera.GetTransform().GetForward());
	gClientCore->Send(packetCamera);
}

void Scene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	MeshLoader Loader{};
	MeshData data{}; 

	mMeshMap["Cube"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Cube, 1);

	data = Loader.Load("Resources/Assets/Knight/BaseAnim/BaseAnim.gltf");
	mMeshMap["HumanBaseAnim"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Knight/LongSword/SwordMan.glb");
	mMeshMap["SwordMan"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Demon/Demon.glb");
	mMeshMap["Demon"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Mountain/Mountain.gltf");
	mMeshMap["Mountain"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Mountain"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/Mountain/Mountain1.glb");
	mMeshMap["Mountain1"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Mountain1"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/imp/imp.glb");
	mMeshMap["MonsterType1"] = std::make_unique<Mesh>(device, commandList, data);
	
	data = Loader.Load("Resources/Assets/Weapon/sword/LongSword.glb");
	mMeshMap["Sword"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["Sword"] = Collider{ data.position };

	data = Loader.Load("Resources/Assets/CorruptedGem/CorruptedGem.glb");
	mMeshMap["CorruptedGem"] = std::make_unique<Mesh>(device, commandList, data);
	mColliderMap["CorruptedGem"] = Collider{ data.position };


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

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Paladin_diffuse");
	mMaterialManager->CreateMaterial("CubeMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("Default_OBJ_baseColor");
	mMaterialManager->CreateMaterial("MountainMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("rock_base_color");
	mMaterialManager->CreateMaterial("Mountain1Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("SwordA_v004_Default_AlbedoTransparency");
	mMaterialManager->CreateMaterial("SwordMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("T_Demon_Imp_Monster_Bloody_Albedo_Skin_4");
	mMaterialManager->CreateMaterial("MonsterType1Material", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("CorrupedGem_BaseColor");
	mMaterialManager->CreateMaterial("CorruptedGemMaterial", mat);

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("T_BigDemonWarrior_Body_Albedo_Skin_3");
	mMaterialManager->CreateMaterial("DemonMaterial", mat);

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
	Scene::BuildAxeAnimationController(); 

	Scene::BuildMonsterType1AnimationController();
	Scene::BuildDemonAnimationController(); 



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

	GameObject baseMountain1;
	baseMountain1.mMesh = mMeshMap["Mountain1"].get();
	baseMountain1.mShader = mShaderMap["StandardShader"].get();
	baseMountain1.mMaterial = mMaterialManager->GetMaterial("Mountain1Material");
	baseMountain1.mCollider = mColliderMap["Mountain1"];

	GameObject baseMountain2;
	baseMountain2.mMesh = mMeshMap["Mountain3"].get();
	baseMountain2.mShader = mShaderMap["StandardShader"].get();
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

	


	// 
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
			object = baseTimberHouse.Clone();
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

		AnimatorGraph::BoneMaskAnimationState forwardState{};
		forwardState.maskedClipIndex = 1;
		forwardState.nonMaskedClipIndex = 1;
		forwardState.speed = 0.7;
		forwardState.name = "Forward";

		AnimatorGraph::BoneMaskAnimationState backwardState{};
		backwardState.maskedClipIndex = 2;
		backwardState.nonMaskedClipIndex = 2;
		backwardState.speed = 1.2;
		backwardState.name = "BackWard";

		AnimatorGraph::BoneMaskAnimationState leftState{};
		leftState.maskedClipIndex = 4;
		leftState.nonMaskedClipIndex = 4;
		leftState.speed = 0.7;
		leftState.name = "Left";

		AnimatorGraph::BoneMaskAnimationState rightState{};
		rightState.maskedClipIndex = 3;
		rightState.nonMaskedClipIndex = 3;
		rightState.speed = 0.7;
		rightState.name = "Right";


		AnimatorGraph::BoneMaskAnimationState jumpState{};
		jumpState.maskedClipIndex = 5;
		jumpState.nonMaskedClipIndex = 5;
		jumpState.name = "Jump";


		mBaseAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, { idleState, forwardState, backwardState, leftState, rightState, jumpState });

	}
}

void Scene::BuildArcherAnimationController() {
	{
		mAnimationMap["Archer"].Load("Resources/Assets/Knight/Archer/Archer.glb");
		auto& loader = mAnimationMap["Archer"];

		std::vector<const AnimationClip*> clips{
			loader.GetClip(0), // Idle
			loader.GetClip(1), // Forward Run
			loader.GetClip(2), // Backward Run
			loader.GetClip(3), // Right Run
			loader.GetClip(4), // Left Run
			loader.GetClip(5), // Jump 
			loader.GetClip(6), // Attacked
			loader.GetClip(7), // Attack Ready 
			loader.GetClip(8), // Attack Shoot	
			loader.GetClip(9), // Interaction 
			loader.GetClip(10), // Death 
		};

		std::vector<UINT> boneMask(67);
		std::iota(boneMask.begin(), boneMask.begin() + 57, 0);

		AnimatorGraph::BoneMaskAnimationState idleState{};
		idleState.maskedClipIndex = 0;
		idleState.nonMaskedClipIndex = 0;
		idleState.name = "Idle";

		AnimatorGraph::BoneMaskAnimationState forwardState{};
		forwardState.maskedClipIndex = 1;
		forwardState.nonMaskedClipIndex = 1;
		forwardState.name = "Forward";
		forwardState.speed = 1.2;

		AnimatorGraph::BoneMaskAnimationState backwardState{};
		backwardState.maskedClipIndex = 2;
		backwardState.nonMaskedClipIndex = 2;
		backwardState.name = "BackWard";

		AnimatorGraph::BoneMaskAnimationState leftState{};
		leftState.maskedClipIndex = 3;
		leftState.nonMaskedClipIndex = 3;
		leftState.name = "Left";

		AnimatorGraph::BoneMaskAnimationState rightState{};
		rightState.maskedClipIndex = 4;
		rightState.nonMaskedClipIndex = 4;
		rightState.name = "Right";

		AnimatorGraph::BoneMaskAnimationState jumpState{};
		jumpState.maskedClipIndex = 5;
		jumpState.nonMaskedClipIndex = 5;
		jumpState.name = "Jump";
		jumpState.loop = false;

		AnimatorGraph::BoneMaskAnimationState attackedState{};
		attackedState.maskedClipIndex = 6;
		attackedState.nonMaskedClipIndex = 6;
		attackedState.name = "Attacked";
		attackedState.loop = false;

		AnimatorGraph::BoneMaskAnimationState attackState{};
		attackedState.maskedClipIndex = 8;
		attackedState.nonMaskedClipIndex = 8;
		attackedState.name = "Attack";
		attackedState.loop = true;

		AnimatorGraph::BoneMaskAnimationState interactionState{};
		interactionState.maskedClipIndex = 9;
		interactionState.nonMaskedClipIndex = 9;
		interactionState.name = "Interaction";
		interactionState.loop = true;

		AnimatorGraph::BoneMaskAnimationState deathState{};
		deathState.maskedClipIndex = 10;
		deathState.nonMaskedClipIndex = 10;
		deathState.name = "Death";
		deathState.loop = false;

		std::vector<AnimatorGraph::BoneMaskAnimationState> states{ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState };
		mArcherAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
	}
}

void Scene::BuildSwordManAnimationController() {
	// LongSword
	{
		mAnimationMap["LongSword"].Load("Resources/Assets/Knight/LongSword/SwordMan.glb");
		auto& loader = mAnimationMap["LongSword"];

		std::vector<const AnimationClip*> clips{
			loader.GetClip(0), // Idle
			loader.GetClip(1), // Forward Run
			loader.GetClip(2), // Backward Run
			loader.GetClip(3), // Right Run
			loader.GetClip(4), // Left Run
			loader.GetClip(5), // Jump 
			loader.GetClip(6), // Attacked
			loader.GetClip(7), // Attack 
			loader.GetClip(8), // Interaction 
			loader.GetClip(9), // Death 
		};

		std::vector<UINT> boneMask(67);
		std::iota(boneMask.begin(), boneMask.begin() + 57, 0);

		AnimatorGraph::BoneMaskAnimationState idleState{};
		idleState.maskedClipIndex = 0;
		idleState.nonMaskedClipIndex = 0;
		idleState.name = "Idle";

		AnimatorGraph::BoneMaskAnimationState forwardState{};
		forwardState.maskedClipIndex = 1;
		forwardState.nonMaskedClipIndex = 1;
		forwardState.name = "Forward";
		forwardState.speed = 1.2;

		AnimatorGraph::BoneMaskAnimationState backwardState{};
		backwardState.maskedClipIndex = 2;
		backwardState.nonMaskedClipIndex = 2;
		backwardState.name = "BackWard";

		AnimatorGraph::BoneMaskAnimationState leftState{};
		leftState.maskedClipIndex = 3;
		leftState.nonMaskedClipIndex = 3;
		leftState.name = "Left";

		AnimatorGraph::BoneMaskAnimationState rightState{};
		rightState.maskedClipIndex = 4;
		rightState.nonMaskedClipIndex = 4;
		rightState.name = "Right";

		AnimatorGraph::BoneMaskAnimationState jumpState{};
		jumpState.maskedClipIndex = 5;
		jumpState.nonMaskedClipIndex = 5;
		jumpState.name = "Jump";
		jumpState.loop = false;

		AnimatorGraph::BoneMaskAnimationState attackedState{};
		attackedState.maskedClipIndex = 6;
		attackedState.nonMaskedClipIndex = 6;
		attackedState.name = "Attacked";
		attackedState.loop = false;
		
		AnimatorGraph::BoneMaskAnimationState attackState{};
		attackedState.maskedClipIndex = 7;
		attackedState.nonMaskedClipIndex = 7;
		attackedState.name = "Attack";
		attackedState.loop = true;

		AnimatorGraph::BoneMaskAnimationState interactionState{};
		interactionState.maskedClipIndex = 8;
		interactionState.nonMaskedClipIndex = 8;
		interactionState.name = "Interaction";
		interactionState.loop = true;

		AnimatorGraph::BoneMaskAnimationState deathState{};
		deathState.maskedClipIndex = 9;
		deathState.nonMaskedClipIndex = 9;
		deathState.name = "Death";
		deathState.loop = false;

		std::vector<AnimatorGraph::BoneMaskAnimationState> states{ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState };
		mSwordManAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
	}
}

void Scene::BuildMageAnimationController() {
	{
		mAnimationMap["Magician"].Load("Resources/Assets/Knight/Mage/Magician.glb");
		auto& loader = mAnimationMap["Magician"];

		std::vector<const AnimationClip*> clips{
			loader.GetClip(0), // Idle
			loader.GetClip(1), // Forward Run
			loader.GetClip(2), // Backward Run
			loader.GetClip(3), // Right Run
			loader.GetClip(4), // Left Run
			loader.GetClip(5), // Jump 
			loader.GetClip(6), // Attacked
			loader.GetClip(7), // Attack 
			loader.GetClip(8), // Interaction 
			loader.GetClip(9), // Death 
		};

		std::vector<UINT> boneMask(67);
		std::iota(boneMask.begin(), boneMask.begin() + 57, 0);

		AnimatorGraph::BoneMaskAnimationState idleState{};
		idleState.maskedClipIndex = 0;
		idleState.nonMaskedClipIndex = 0;
		idleState.name = "Idle";

		AnimatorGraph::BoneMaskAnimationState forwardState{};
		forwardState.maskedClipIndex = 1;
		forwardState.nonMaskedClipIndex = 1;
		forwardState.name = "Forward";
		forwardState.speed = 1.2;

		AnimatorGraph::BoneMaskAnimationState backwardState{};
		backwardState.maskedClipIndex = 2;
		backwardState.nonMaskedClipIndex = 2;
		backwardState.name = "BackWard";

		AnimatorGraph::BoneMaskAnimationState leftState{};
		leftState.maskedClipIndex = 3;
		leftState.nonMaskedClipIndex = 3;
		leftState.name = "Left";

		AnimatorGraph::BoneMaskAnimationState rightState{};
		rightState.maskedClipIndex = 4;
		rightState.nonMaskedClipIndex = 4;
		rightState.name = "Right";

		AnimatorGraph::BoneMaskAnimationState jumpState{};
		jumpState.maskedClipIndex = 5;
		jumpState.nonMaskedClipIndex = 5;
		jumpState.name = "Jump";
		jumpState.loop = false;

		AnimatorGraph::BoneMaskAnimationState attackedState{};
		attackedState.maskedClipIndex = 6;
		attackedState.nonMaskedClipIndex = 6;
		attackedState.name = "Attacked";
		attackedState.loop = false;

		AnimatorGraph::BoneMaskAnimationState attackState{};
		attackedState.maskedClipIndex = 7;
		attackedState.nonMaskedClipIndex = 7;
		attackedState.name = "Attack";
		attackedState.loop = true;

		AnimatorGraph::BoneMaskAnimationState interactionState{};
		interactionState.maskedClipIndex = 8;
		interactionState.nonMaskedClipIndex = 8;
		interactionState.name = "Interaction";
		interactionState.loop = true;

		AnimatorGraph::BoneMaskAnimationState deathState{};
		deathState.maskedClipIndex = 9;
		deathState.nonMaskedClipIndex = 9;
		deathState.name = "Death";
		deathState.loop = false;

		std::vector<AnimatorGraph::BoneMaskAnimationState> states{ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState };
		mMageAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
	}
}

void Scene::BuildAxeAnimationController() {
	mAnimationMap["Axe"].Load("Resources/Assets/Knight/Axe/Axe.glb");
	auto& loader = mAnimationMap["Axe"];

	std::vector<const AnimationClip*> clips{
		loader.GetClip(0), // Idle
		loader.GetClip(1), // Forward Run
		loader.GetClip(2), // Backward Run
		loader.GetClip(3), // Right Run
		loader.GetClip(4), // Left Run
		loader.GetClip(5), // Jump 
		loader.GetClip(6), // Attacked
		loader.GetClip(7), // Attack 
		loader.GetClip(8), // Interaction 
		loader.GetClip(9), // Death 
	};

	std::vector<UINT> boneMask(67);
	std::iota(boneMask.begin(), boneMask.begin() + 57, 0);

	AnimatorGraph::BoneMaskAnimationState idleState{};
	idleState.maskedClipIndex = 0;
	idleState.nonMaskedClipIndex = 0;
	idleState.name = "Idle";

	AnimatorGraph::BoneMaskAnimationState forwardState{};
	forwardState.maskedClipIndex = 1;
	forwardState.nonMaskedClipIndex = 1;
	forwardState.name = "Forward";
	forwardState.speed = 1.2;

	AnimatorGraph::BoneMaskAnimationState backwardState{};
	backwardState.maskedClipIndex = 2;
	backwardState.nonMaskedClipIndex = 2;
	backwardState.name = "BackWard";

	AnimatorGraph::BoneMaskAnimationState leftState{};
	leftState.maskedClipIndex = 3;
	leftState.nonMaskedClipIndex = 3;
	leftState.name = "Left";

	AnimatorGraph::BoneMaskAnimationState rightState{};
	rightState.maskedClipIndex = 4;
	rightState.nonMaskedClipIndex = 4;
	rightState.name = "Right";

	AnimatorGraph::BoneMaskAnimationState jumpState{};
	jumpState.maskedClipIndex = 5;
	jumpState.nonMaskedClipIndex = 5;
	jumpState.name = "Jump";
	jumpState.loop = false;

	AnimatorGraph::BoneMaskAnimationState attackedState{};
	attackedState.maskedClipIndex = 6;
	attackedState.nonMaskedClipIndex = 6;
	attackedState.name = "Attacked";
	attackedState.loop = false;

	AnimatorGraph::BoneMaskAnimationState attackState{};
	attackedState.maskedClipIndex = 7;
	attackedState.nonMaskedClipIndex = 7;
	attackedState.name = "Attack";
	attackedState.loop = true;

	AnimatorGraph::BoneMaskAnimationState interactionState{};
	interactionState.maskedClipIndex = 8;
	interactionState.nonMaskedClipIndex = 8;
	interactionState.name = "Interaction";
	interactionState.loop = true;

	AnimatorGraph::BoneMaskAnimationState deathState{};
	deathState.maskedClipIndex = 9;
	deathState.nonMaskedClipIndex = 9;
	deathState.name = "Death";
	deathState.loop = false;

	std::vector<AnimatorGraph::BoneMaskAnimationState> states{ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState };
	mAxeAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
}

void Scene::BuildMonsterType1AnimationController() {
	mAnimationMap["MonsterType1"].Load("Resources/Assets/imp/imp.glb");
	auto& loader = mAnimationMap["MonsterType1"];

	AnimatorGraph::AnimationState idleState{};
	idleState.clip = loader.GetClip(0);
	idleState.name = "Idle";
	idleState.loop = true;

	AnimatorGraph::AnimationState forwardState{};
	forwardState.clip = loader.GetClip(1);
	forwardState.name = "Forward";
	forwardState.loop = true;

	AnimatorGraph::AnimationState backwardState{};
	backwardState.clip = loader.GetClip(2);
	backwardState.name = "BackWard";
	backwardState.loop = true;

	AnimatorGraph::AnimationState leftState{};
	leftState.clip = loader.GetClip(3);
	leftState.name = "Left";
	leftState.loop = true;

	AnimatorGraph::AnimationState rightState{};
	rightState.clip = loader.GetClip(4);
	rightState.name = "Right";
	rightState.loop = true;

	AnimatorGraph::AnimationState jumpState{};
	jumpState.clip = loader.GetClip(5);
	jumpState.name = "Jump";
	jumpState.loop = false;

	AnimatorGraph::AnimationState attackedState{};
	attackedState.clip = loader.GetClip(6);
	attackedState.name = "Attacked";
	attackedState.loop = false;

	AnimatorGraph::AnimationState attackState{};
	attackState.clip = loader.GetClip(7);
	attackState.name = "Attack";
	attackState.loop = false;

	AnimatorGraph::AnimationState interactionState{};
	interactionState.clip = loader.GetClip(0);
	interactionState.name = "Interaction";
	interactionState.loop = true;

	AnimatorGraph::AnimationState deathState{};
	deathState.clip = loader.GetClip(8);
	deathState.name = "Death";
	deathState.loop = false;

	mMonsterType1AnimationController = AnimatorGraph::AnimationGraphController({ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState });

}

void Scene::BuildDemonAnimationController() {
	mAnimationMap["Demon"].Load("Resources/Assets/Demon/Demon.glb");
	auto& loader = mAnimationMap["Demon"];

	AnimatorGraph::AnimationState idleState{};
	idleState.clip = loader.GetClip(0);
	idleState.name = "Idle";
	idleState.loop = true;

	AnimatorGraph::AnimationState forwardState{};
	forwardState.clip = loader.GetClip(1);
	forwardState.name = "Forward";
	forwardState.loop = true;

	AnimatorGraph::AnimationState backwardState{};
	backwardState.clip = loader.GetClip(2);
	backwardState.name = "BackWard";
	backwardState.loop = true;

	AnimatorGraph::AnimationState leftState{};
	leftState.clip = loader.GetClip(3);
	leftState.name = "Left";
	leftState.loop = true;

	AnimatorGraph::AnimationState rightState{};
	rightState.clip = loader.GetClip(4);
	rightState.name = "Right";
	rightState.loop = true;

	AnimatorGraph::AnimationState jumpState{};
	jumpState.clip = loader.GetClip(5);
	jumpState.name = "Jump";
	jumpState.loop = false;

	AnimatorGraph::AnimationState attackedState{};
	attackedState.clip = loader.GetClip(6);
	attackedState.name = "Attacked";
	attackedState.loop = false;

	AnimatorGraph::AnimationState attackState{};
	attackState.clip = loader.GetClip(7);
	attackState.name = "Attack";
	attackState.loop = false;

	AnimatorGraph::AnimationState interactionState{};
	interactionState.clip = loader.GetClip(0);
	interactionState.name = "Interaction";
	interactionState.loop = true;

	AnimatorGraph::AnimationState deathState{};
	deathState.clip = loader.GetClip(8);
	deathState.name = "Death";
	deathState.loop = false;

	mDemonAnimationController = AnimatorGraph::AnimationGraphController({ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState });
}
