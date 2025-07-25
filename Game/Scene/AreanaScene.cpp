#include "pch.h"
#include "AreanaScene.h"
#include "../Renderer/Core/Renderer.h"
#include "../MeshLoader/Loader/MeshLoader.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#ifdef _DEBUG
#pragma comment(lib,"out/debug/MeshLoader.lib")
#else 
#pragma comment(lib,"out/release/MeshLoader.lib")
#endif
#include "../Utility/NonReplacementSampler.h"
#include "../ServerLib/GameProtocol.h"
#include "../Renderer/Core/Console.h"



void ArenaScene::ProcessPlayerExit(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::PlayerExitSC>(buffer);
	if (mPlayerIndexmap.contains(data->playerId())) {
		mPlayerIndexmap[data->playerId()]->SetActiveState(false);
		mPlayerIndexmap[data->playerId()]->SetEmpty(true);
	}
}

void ArenaScene::ProcessLatency(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::PacketLatencySC>(buffer);

	auto now = std::chrono::steady_clock::now();
	auto old = std::chrono::time_point<std::chrono::steady_clock>(std::chrono::nanoseconds(data->latency()));

	mLatency[mLatencySampleIndex] = std::chrono::duration_cast<duration>(now - old);
	mLatencySampleIndex = (mLatencySampleIndex + 1) % mLatency.size();

	mAvgLatency = GetAverageLatency<std::chrono::seconds>();
}

void ArenaScene::ProcessObjectAppeared(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectAppearedSC>(buffer);

	auto FindNextPlayerLoc = [this]() {
		for (auto iter = mPlayers.begin(); iter != mPlayers.end(); ++iter) {
			if (iter->GetEmpty()) {
				return iter;
			}
		}
		return mPlayers.end();
		};

	auto FindNextObjectLoc = [this]() {
		for (auto iter = mGameObjects.begin(); iter != mGameObjects.end(); ++iter) {
			if (iter->GetEmpty()) {
				return iter;
			}
		}
		return mGameObjects.end();
		};

	// 플레이어 등장 
	if (data->objectId() < OBJECT_ID_START) {
		// 내 플레이어 등장 
		if (data->objectId() == gClientCore2->GetSessionId()) {
			// 플레이어 인스턴스가 없다면 
			if (mMyPlayer == nullptr) {

				auto nextLoc = FindNextPlayerLoc();

				if (nextLoc == mPlayers.end()) {
					MessageBox(nullptr, L"ERROR!!!!!\nThere is no more space for My Player!!", L"", MB_OK | MB_ICONERROR);
					Crash("There is no more space for My Player!!");
				}

				SimpleMath::Vector3 cameraOffset{ 0.f, 1.75f, 3.f };

				switch (data->entity()) {
				case Packets::EntityType_HUMAN_LONGSWORD:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedNormalShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mSwordManAnimationController);
					nextLoc->AddEquipment(mEquipments["GreatSword"].Clone());
					mProfileUI.Init(mRenderManager->GetCanvas(), mRenderManager->GetTextureManager().GetTexture("big_circle_frame"), mRenderManager->GetTextureManager().GetTexture("GreatSword"));
					break;
				case Packets::EntityType_HUMAN_SWORD:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedNormalShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mShieldManController);
					nextLoc->AddEquipment(mEquipments["Sword"].Clone());
					nextLoc->AddEquipment(mEquipments["Shield"].Clone());
					mProfileUI.Init(mRenderManager->GetCanvas(), mRenderManager->GetTextureManager().GetTexture("big_circle_frame"), mRenderManager->GetTextureManager().GetTexture("ShieldMan"));
					break;
				case Packets::EntityType_HUMAN_ARCHER:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedNormalShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mArcherAnimationController);
					nextLoc->AddEquipment(mEquipments["Bow"].Clone());
					nextLoc->AddEquipment(mEquipments["Quiver"].Clone());
					mProfileUI.Init(mRenderManager->GetCanvas(), mRenderManager->GetTextureManager().GetTexture("big_circle_frame"), mRenderManager->GetTextureManager().GetTexture("Archer"));
					break;
				case Packets::EntityType_HUMAN_MAGICIAN:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedNormalShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mMageAnimationController);
					nextLoc->AddEquipment(mEquipments["Staff"].Clone());
					mProfileUI.Init(mRenderManager->GetCanvas(), mRenderManager->GetTextureManager().GetTexture("big_circle_frame"), mRenderManager->GetTextureManager().GetTexture("Magician"));
					break;
				case Packets::EntityType_BOSS:
					*nextLoc = Player(mMeshMap["Demon"].get(), mShaderMap["SkinnedNormalShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("DemonMaterial"), mDemonAnimationController);
					nextLoc->AddEquipment(mEquipments["DemonWeapon"].Clone());
					nextLoc->AddEquipment(mEquipments["DemonCloth"].Clone());
					cameraOffset *= 2.f;
					mProfileUI.Init(mRenderManager->GetCanvas(), mRenderManager->GetTextureManager().GetTexture("big_circle_frame"), mRenderManager->GetTextureManager().GetTexture("Devil"));
					break;
				default:
					MessageBox(nullptr, L"Something went wrong!!", L"", MB_OK | MB_ICONERROR);
					break;
				}

				mPlayerIndexmap[data->objectId()] = &(*nextLoc);
				mMyPlayer = &(*nextLoc);

				mMyPlayer->SetMyPlayer();

				mMyPlayer->GetTransform().GetPosition() = FbsPacketFactory::GetVector3(data->pos());

				mMyPlayer->SetAnimation(data->animation());

				mHealthBarUI.SetHealth(data->hp());
				mFreeCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
				mTPPCameraMode = std::make_unique<TPPCameraMode>(&mCamera, mMyPlayer->GetTransform(), cameraOffset);

				mCurrentCameraMode = mTPPCameraMode.get();

				mCurrentCameraMode->Enter();




#ifdef DEV_MODE
				int sign = NonReplacementSampler::GetInstance().Sample();

				Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::P, sign, [this]() {
					mCurrentCameraMode->Exit();
					if (mCurrentCameraMode == mTPPCameraMode.get()) {
						mCurrentCameraMode = mFreeCameraMode.get();
					}
					else {
						mCurrentCameraMode = mTPPCameraMode.get();
					}

					mCurrentCameraMode->Enter();
					});
#endif 

			}
			else {
				if (mPlayerIndexmap[data->objectId()] != nullptr) {
					auto& player = *mPlayerIndexmap[data->objectId()];
					player.SetActiveState(true);

					auto zxPos = FbsPacketFactory::GetVector3(data->pos());
					zxPos.y = 0.f;

					player.GetTransform().GetPosition() = zxPos;
					player.SetAnimation(data->animation());
					player.GetTransform().ResetPrediction();
				}
			}
		}
		// 다른 플레이어 등장	
		else {
			// 그 플레이어 인스턴스가 없다면  
			if (not mPlayerIndexmap.contains(data->objectId())) {

				auto nextLoc = FindNextPlayerLoc();
				if (nextLoc == mPlayers.end()) {
					MessageBox(nullptr, L"ERROR!!!!!\nThere is no more space for Other Player!!", L"", MB_OK | MB_ICONERROR);
					Crash("There is no more space for Other Player!!");
				}


				switch (data->entity()) {
				case Packets::EntityType_HUMAN_LONGSWORD:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mSwordManAnimationController);
					nextLoc->AddEquipment(mEquipments["GreatSword"].Clone());

					break;
				case Packets::EntityType_HUMAN_SWORD:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mShieldManController);
					nextLoc->AddEquipment(mEquipments["Sword"].Clone());
					nextLoc->AddEquipment(mEquipments["Shield"].Clone());

					break;
				case Packets::EntityType_HUMAN_ARCHER:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mArcherAnimationController);
					nextLoc->AddEquipment(mEquipments["Bow"].Clone());
					nextLoc->AddEquipment(mEquipments["Quiver"].Clone());

					break;
				case Packets::EntityType_HUMAN_MAGICIAN:
					*nextLoc = Player(mMeshMap["SwordMan"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial"), mMageAnimationController);
					nextLoc->AddEquipment(mEquipments["Staff"].Clone());
					break;
				case Packets::EntityType_BOSS:
					*nextLoc = Player(mMeshMap["Demon"].get(), mShaderMap["SkinnedShader"].get(), mRenderManager->GetMaterialManager().GetMaterial("DemonMaterial"), mDemonAnimationController);
					nextLoc->AddEquipment(mEquipments["DemonWeapon"].Clone());
					nextLoc->AddEquipment(mEquipments["DemonCloth"].Clone());

					break;
				default:
					MessageBox(nullptr, L"Something went wrong!!", L"", MB_OK | MB_ICONERROR);
					break;
				}


				mPlayerIndexmap[data->objectId()] = &(*nextLoc);

				nextLoc->GetTransform().GetPosition() = FbsPacketFactory::GetVector3(data->pos());
				nextLoc->SetAnimation(data->animation());

			}
			else {
				if (mPlayerIndexmap[data->objectId()] != nullptr) {
					auto& player = *mPlayerIndexmap[data->objectId()];
					player.SetActiveState(true);

					auto zxPos = FbsPacketFactory::GetVector3(data->pos());
					zxPos.y = 0.f;

					player.GetTransform().GetPosition() = zxPos;
					player.SetAnimation(data->animation());
					player.GetTransform().ResetPrediction();
				}
			}
		}
	}
	// 이외 오브젝트 등장 
	else {
		if (not mGameObjectMap.contains(data->objectId())) {
			auto nextLoc = FindNextObjectLoc();

			if (nextLoc == mGameObjects.end()) {
				MessageBox(nullptr, L"ERROR!!!!!\nThere is no more space for Other Object!!", L"", MB_OK | MB_ICONERROR);
				Crash("There is no more space for Other Object!!");
			}

			switch (data->entity()) {
			case Packets::EntityType_MONSTER:
			{
				*nextLoc = GameObject{};
				mGameObjectMap[data->objectId()] = &(*nextLoc);

				nextLoc->mShader = mShaderMap["SkinnedNormalShader"].get();
				nextLoc->mMesh = mMeshMap["MonsterType1"].get();
				nextLoc->mMaterial = mRenderManager->GetMaterialManager().GetMaterial("MonsterType1Material");
				nextLoc->mGraphController = mMonsterAnimationController;
				nextLoc->mCollider = mColliderMap["MonsterType1"];
				nextLoc->mAnimated = true;
				nextLoc->SetActiveState(true);


				nextLoc->GetTransform().SetPosition(FbsPacketFactory::GetVector3(data->pos()));
				nextLoc->mGraphController.Transition(static_cast<size_t>(data->animation()));

				nextLoc->SetEmpty(false);
			}
			break;
			case Packets::EntityType_CORRUPTED_GEM:
			{
				*nextLoc = GameObject{};
				mGameObjectMap[data->objectId()] = &(*nextLoc);
				nextLoc->mShader = mShaderMap["StandardShader"].get();
				nextLoc->mMesh = mMeshMap["CorruptedGem"].get();
				nextLoc->mMaterial = mRenderManager->GetMaterialManager().GetMaterial("CorruptedGemMaterial");
				nextLoc->SetActiveState(true);


				nextLoc->GetTransform().SetPosition(FbsPacketFactory::GetVector3(data->pos()));
				nextLoc->GetTransform().GetPosition().y = 0.f;

				nextLoc->SetEmpty(false);

				ParticleVertex v{};
				v.position = nextLoc->GetTransform().GetPosition();

				v.halfheight = 10.f;
				v.halfWidth = 10.f;
				v.material = mRenderManager->GetMaterialManager().GetMaterial("SmokeMaterial");
				v.spritable = true;
				v.spriteDuration = 1.f;
				v.spriteFrameInRow = 4;
				v.spriteFrameInCol = 4;
				v.direction = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
				v.velocity = { 0.f, 0.f, 0.f };
				v.totalLifeTime = 0.5f;
				v.lifeTime = 0.5f;
				v.type = ParticleType_emit;
				v.emitType = ParticleType_smoke;
				v.remainEmit = 100000;
				v.emitIndex = 0;

				mParticleMap[data->objectId()] = mRenderManager->GetParticleManager().CreateEmitParticle(v);
				mParticleMap[data->objectId()].Get()->position = v.position;
			}
			break;
			case Packets::EntityType_ITEM_POTION:
			{
			}
			break;
			default:
				break;
			}
		}
		else {
			if (mGameObjectMap[data->objectId()] != nullptr) {
				auto& object = *mGameObjectMap[data->objectId()];
				object.SetActiveState(true);

				auto zxPos = FbsPacketFactory::GetVector3(data->pos());
				zxPos.y = 0.f; 

				object.GetTransform().GetPosition() = zxPos;
				object.GetTransform().ResetPrediction();

				if (object.mAnimated) {
					object.mGraphController.Transition(static_cast<size_t>(data->animation()));
				}

			}
		}

	}
}

void ArenaScene::ProcessObjectDisappeared(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectDisappearedSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {
			mPlayerIndexmap[data->objectId()]->GetTransform().ResetPrediction();
			mPlayerIndexmap[data->objectId()]->SetActiveState(false);
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->GetTransform().ResetPrediction();
			mGameObjectMap[data->objectId()]->SetActiveState(false);
		}
	}
}

void ArenaScene::ProcessObjectRemoved(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectRemovedSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {
			mPlayerIndexmap[data->objectId()]->SetActiveState(false);
			mPlayerIndexmap[data->objectId()]->SetEmpty(true);
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->SetActiveState(false);
			mGameObjectMap[data->objectId()]->SetEmpty(true);
		}
	}
}

void ArenaScene::ProcessObjectMove(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectMoveSC>(buffer);

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {
			float predictDuration = mAvgLatency + data->duration() + 0.3f;

			auto zxPos = FbsPacketFactory::GetVector3(data->pos());
			zxPos.y = 0.f;

			mPlayerIndexmap[data->objectId()]->GetTransform().SetPrediction(zxPos, predictDuration);


			if (data->objectId() == gClientCore2->GetSessionId()) {
				return;
			}

			auto euler = mPlayerIndexmap[data->objectId()]->GetTransform().GetRotation().ToEuler();
			euler.y = data->yaw();
			mPlayerIndexmap[data->objectId()]->GetTransform().GetRotation() = SimpleMath::Quaternion::CreateFromYawPitchRoll(euler.y, euler.x, euler.z);
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			float predictDuration = mAvgLatency + data->duration();

			auto zxPos = FbsPacketFactory::GetVector3(data->pos());
			zxPos.y = 0.f;

			mGameObjectMap[data->objectId()]->GetTransform().SetPrediction(zxPos, predictDuration);

			auto euler = mGameObjectMap[data->objectId()]->GetTransform().GetRotation().ToEuler();
			euler.y = data->yaw();

			mGameObjectMap[data->objectId()]->GetTransform().GetRotation() = SimpleMath::Quaternion::CreateFromYawPitchRoll(euler.y, euler.x, euler.z);
		}
	}
}

void ArenaScene::ProcessObjectAttacked(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectAttackedSC>(buffer);
	// HP 깍기 
	if (data->objectId() == gClientCore2->GetSessionId()) {
		mHealthBarUI.SetHealth(data->hp());

		if (data->hp() <= MathUtil::EPSILON and mMyPlayer != nullptr) {
			mMyPlayer->LockRotate(true);
		}

	}
}

void ArenaScene::ProcessPacketAnimation(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::ObjectAnimationChangedSC>(buffer);

	const float PrimaryVolume = 5.f;
	const float SoundDistance = 10.f;

	if (data->objectId() < OBJECT_ID_START) {
		if (mPlayerIndexmap.contains(data->objectId())) {

			if (data->objectId() == gClientCore2->GetSessionId()) {
				if (data->animation() == Packets::AnimationState_ATTACK) {
					mMyPlayer->LockRotate(true);

					switch (mMyPlayer->GetMyRole()) {
					case Packets::EntityType_HUMAN_LONGSWORD:
						SoundManager::GetInstance().PlaySound(std::string{ "SwordSlash" }, PrimaryVolume, SoundOption::NONE, 0ms, 250ms);
						break;
					case Packets::EntityType_HUMAN_SWORD:
						SoundManager::GetInstance().PlaySound(std::string{ "SwordSlash" }, PrimaryVolume, SoundOption::NONE, 0ms, 250ms);
						break;
					case Packets::EntityType_HUMAN_ARCHER:
					{
						UINT type = RandomEngine::GetRandomRange(1U, 3U);
						SoundManager::GetInstance().PlaySound(std::string{ "BowPullAndRelease" } + std::to_string(type), PrimaryVolume, SoundOption::NONE, 0ms, 200ms);
					}
					break;
					case Packets::EntityType_HUMAN_MAGICIAN:
						break;
					default:
						break;
					}
				}
				else if (data->animation() == Packets::AnimationState_DEAD) {
					mMyPlayer->LockRotate(true);

					switch (mMyPlayer->GetMyRole()) {
					case Packets::EntityType_HUMAN_LONGSWORD:
					case Packets::EntityType_HUMAN_ARCHER:
					case Packets::EntityType_HUMAN_SWORD:
					case Packets::EntityType_HUMAN_MAGICIAN:
					{
						UINT type = RandomEngine::GetRandomRange(1U, 2U);
						SoundManager::GetInstance().PlaySound(std::string{ "Death" } + std::to_string(type), PrimaryVolume, SoundOption::NONE, 0ms, 0ms);
					}
					break;
					default:
						break;
					}


				}
				else {
					mMyPlayer->LockRotate(false);
				}
			}
			else {
				auto distanceSq = SimpleMath::Vector3::DistanceSquared(mMyPlayer->GetTransform().GetPosition(), mPlayerIndexmap[data->objectId()]->GetTransform().GetPosition());
				if (distanceSq <= SoundDistance * SoundDistance) {
					float volume = 1.0f - std::clamp(std::sqrtf(distanceSq) / SoundDistance, 0.0f, 1.0f);
					if (data->animation() == Packets::AnimationState_ATTACK) {

						switch (mMyPlayer->GetMyRole()) {
						case Packets::EntityType_HUMAN_LONGSWORD:
							SoundManager::GetInstance().PlaySound(std::string{ "SwordSlash" }, PrimaryVolume * volume, SoundOption::NONE, 0ms, 250ms);
							break;
						case Packets::EntityType_HUMAN_SWORD:
							SoundManager::GetInstance().PlaySound(std::string{ "SwordSlash" }, PrimaryVolume * volume, SoundOption::NONE, 0ms, 250ms);
							break;
						case Packets::EntityType_HUMAN_ARCHER:
						{
							UINT type = RandomEngine::GetRandomRange(1U, 3U);
							SoundManager::GetInstance().PlaySound(std::string{ "BowPullAndRelease" } + std::to_string(type), PrimaryVolume * volume, SoundOption::NONE, 0ms, 200ms);
						}
						break;
						case Packets::EntityType_HUMAN_MAGICIAN:
							break;
						default:
							break;
						}
					}
					else if (data->animation() == Packets::AnimationState_DEAD) {
						switch (mMyPlayer->GetMyRole()) {
						case Packets::EntityType_HUMAN_LONGSWORD:
						case Packets::EntityType_HUMAN_ARCHER:
						case Packets::EntityType_HUMAN_SWORD:
						case Packets::EntityType_HUMAN_MAGICIAN:
						{
							UINT type = RandomEngine::GetRandomRange(1U, 2U);
							SoundManager::GetInstance().PlaySound(std::string{ "Death" } + std::to_string(type), PrimaryVolume * volume, SoundOption::NONE, 0ms, 0ms);
						}
						break;
						default:
							break;
						}


					}
				}
			}
			mPlayerIndexmap[data->objectId()]->SetAnimation(data->animation());
		}
	}
	else {
		if (mGameObjectMap.contains(data->objectId())) {
			mGameObjectMap[data->objectId()]->GetAnimationController().Transition(static_cast<size_t>(data->animation()));
		}
	}

}

void ArenaScene::ProcessFireProjectile(const uint8_t* buffer) {
}

void ArenaScene::ProcessProjectileMove(const uint8_t* buffer) {
}

void ArenaScene::ProcessHeartBeat(const uint8_t* buffer) {
	decltype(auto) data = FbsPacketFactory::GetDataPtrSC<Packets::HeartBeatSC>(buffer);
	decltype(auto) packet = FbsPacketFactory::HeartBeatCS(gClientCore2->GetSessionId());
	gClientCore2->Send(packet);
}


ArenaScene::ArenaScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation) {
	mInputSign = NonReplacementSampler::GetInstance().Sample();
	mRenderManager = renderMgr;


	mCamera = Camera(mainCamLocation);
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });
}

ArenaScene::~ArenaScene() {

}

void ArenaScene::Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	ArenaScene::BuildShader(device);
	ArenaScene::BuildMesh(device, commandList);
	ArenaScene::BuildMaterial(); 
	ArenaScene::BuildAniamtionController();

	mLayerIndexMap.LoadFromFile("Resources/Binarys/Terrain/LayerIndex_Arena.bin");

	ArenaScene::BuildEnvironment("Resources/Binarys/Terrain/ArenaSceneObjects.bin");

	for (auto& obj : mEnvironmentObjects) {
		obj.UpdateShaderVariables(); 
	}

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SkyBoxMaterial");

	{
		mEquipments["Sword"] = EquipmentObject{};
		mEquipments["Sword"].mMesh = mMeshMap["Sword"].get();
		mEquipments["Sword"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Sword"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SwordMaterial");
		mEquipments["Sword"].mCollider = mColliderMap["Sword"];
		mEquipments["Sword"].mEquipJointIndex = 36;
		mEquipments["Sword"].SetActiveState(true);
	}


	{
		mEquipments["GreatSword"] = EquipmentObject{};
		mEquipments["GreatSword"].mMesh = mMeshMap["GreatSword"].get();
		mEquipments["GreatSword"].mShader = mShaderMap["StandardNormalShader"].get();
		mEquipments["GreatSword"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("GreatSwordMaterial");
		mEquipments["GreatSword"].mCollider = mColliderMap["GreatSword"];
		mEquipments["GreatSword"].mEquipJointIndex = 36;
		mEquipments["GreatSword"].SetActiveState(true);
	}

	{
		mEquipments["Bow"] = EquipmentObject{};
		mEquipments["Bow"].mMesh = mMeshMap["Bow"].get();
		mEquipments["Bow"].mShader = mShaderMap["StandardNormalShader"].get();
		mEquipments["Bow"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("BowMaterial");
		mEquipments["Bow"].mCollider = mColliderMap["Bow"];
		mEquipments["Bow"].mEquipJointIndex = 12;
		mEquipments["Bow"].SetActiveState(true);
	}

	{
		mEquipments["Quiver"] = EquipmentObject{};
		mEquipments["Quiver"].mMesh = mMeshMap["Quiver"].get();
		mEquipments["Quiver"].mShader = mShaderMap["StandardShader"].get();
		mEquipments["Quiver"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("QuiverMaterial");
		mEquipments["Quiver"].mCollider = mColliderMap["Quiver"];
		mEquipments["Quiver"].mEquipJointIndex = 3;
		mEquipments["Quiver"].SetActiveState(true);
	}

	{
		mEquipments["Shield"] = EquipmentObject{};
		mEquipments["Shield"].mMesh = mMeshMap["Shield"].get();
		mEquipments["Shield"].mShader = mShaderMap["StandardNormalShader"].get();
		mEquipments["Shield"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("CubeMaterial");
		mEquipments["Shield"].mCollider = mColliderMap["Shield"];
		mEquipments["Shield"].mEquipJointIndex = 11;
		mEquipments["Shield"].SetActiveState(true);
	}

	{
		mEquipments["Staff"] = EquipmentObject{};
		mEquipments["Staff"].mMesh = mMeshMap["Staff"].get();
		mEquipments["Staff"].mShader = mShaderMap["StandardNormalShader"].get();
		mEquipments["Staff"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("StaffMaterial");
		mEquipments["Staff"].mCollider = mColliderMap["Staff"];
		mEquipments["Staff"].mEquipJointIndex = 36;
		mEquipments["Staff"].SetActiveState(true);
	}


	{
		mEquipments["DemonCloth"] = EquipmentObject{};
		mEquipments["DemonCloth"].mMesh = mMeshMap["DemonCloth"].get();
		mEquipments["DemonCloth"].mShader = mShaderMap["StandardNormalShader"].get();
		mEquipments["DemonCloth"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("DemonClothMaterial");
		mEquipments["DemonCloth"].mCollider = mColliderMap["DemonCloth"];
		mEquipments["DemonCloth"].mEquipJointIndex = 0;
		mEquipments["DemonCloth"].SetActiveState(true);
	}

	{
		mEquipments["DemonWeapon"] = EquipmentObject{};
		mEquipments["DemonWeapon"].mMesh = mMeshMap["DemonWeapon"].get();
		mEquipments["DemonWeapon"].mShader = mShaderMap["StandardNormalShader"].get();
		mEquipments["DemonWeapon"].mMaterial = mRenderManager->GetMaterialManager().GetMaterial("DemonWeaponMaterial");
		mEquipments["DemonWeapon"].mCollider = mColliderMap["DemonWeapon"];
		mEquipments["DemonWeapon"].mEquipJointIndex = 28;
		mEquipments["DemonWeapon"].SetActiveState(true);
	}

	mGameObjects.resize(10000, GameObject{});

	std::weak_ptr<ArenaScene> sharedThis{ std::static_pointer_cast<ArenaScene>(shared_from_this()) };

	Time.AddEvent(66ms, [sharedThis]() {
		if (sharedThis.expired()) {
			return false;
		}
		auto scene = sharedThis.lock();
		scene->SendLook();
		return true;
		});

	Time.AddEvent(500ms, [sharedThis]() {
		if (sharedThis.expired()) {
			return false;
		}

		auto time = std::chrono::steady_clock::now();
		auto packet = FbsPacketFactory::LatencyCS(gClientCore2->GetSessionId(), time.time_since_epoch().count());
		gClientCore2->Send(packet);

		return true;
		});

	Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F6, mInputSign, [this]() {
		mCurrentCameraMode->SetCameraShake(700ms);
	});

	Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F7, mInputSign, [this]() {
		mIsBlind = not mIsBlind;
	});

	decltype(auto) packet = FbsPacketFactory::PlayerEnterInGame(gClientCore2->GetSessionId());
	gClientCore2->Send(packet);

}

void ArenaScene::ProcessNetwork() {
	auto size = gClientCore2->Recv();
	decltype(auto) buffer = gClientCore2->GetBuffer();

	ArenaScene::ProcessPackets(reinterpret_cast<const uint8_t*>(buffer.data()), size);

	gClientCore2->ProcessRemainData(size);
}

void ArenaScene::SendLook() {
	if (not mExpired) {
		auto look = mCamera.GetTransform().GetForward();
		look.y = 0.f;

		decltype(auto) packetCamera = FbsPacketFactory::PlayerLookCS(gClientCore2->GetSessionId(), look);
		gClientCore2->Send(packetCamera);
	}
}

void ArenaScene::UpdateSound() {
	const float footprintDistance = 10.f; // 발자국 소리 들리는 거리 
	const float PrimaryVolume = 5.f;

	// 1. 발자국 소리 ( 플레이어 ) 
	for (auto& pair : mPlayerIndexmap) {
		if (pair.second == nullptr) {
			continue;
		}
		// 나의 경우 
		if (pair.first == gClientCore2->GetSessionId()) {
			// 움직이는 상태라면
			if (pair.second->GetTransform().GetMovingState()) {
				// 이전에 멈춘 상태였다면
				if (mSoundMap[pair.first].second == nullptr) {
					mSoundMap[pair.first].first = mLayerIndexMap.GetLayerIndexAtPosition(pair.second->GetTransform().GetPosition());

					switch (mSoundMap[pair.first].first) {
					case 0: // Grass Area 
						mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("GrassArea", PrimaryVolume, SoundOption::Shuffle, 100ms);
						break;
					case 1: // Load Area
					case 2: // Stone Area 
						mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("DirtArea", PrimaryVolume, SoundOption::Shuffle, 100ms);
						break;
					default:
						break;
					}

				}
				else { // 이전에 움직이고 있던 상태였다면
					UINT currentAreaID{ mLayerIndexMap.GetLayerIndexAtPosition(pair.second->GetTransform().GetPosition()) };


					if (currentAreaID != mSoundMap[pair.first].first) { // Area가 바뀌었다면 
						if (mSoundMap[pair.first].second == nullptr) {
							return; 
						}


						// 먼저 이전 Area 사운드를 빼야 한다. 
						mSoundMap[pair.first].second->Stop();

						mSoundMap[pair.first].first = currentAreaID;

						switch (mSoundMap[pair.first].first) {
						case 0: // Grass Area 
							mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("GrassArea", PrimaryVolume, SoundOption::Shuffle);
							break;
						case 1: // Load Area
						case 2: // Stone Area 
							mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("DirtArea", PrimaryVolume, SoundOption::Shuffle);
							break;
						default:
							break;
						}
					}

				}
			}
			else { // 멈춘 상태라면 
				if (mSoundMap[pair.first].second != nullptr) {
					mSoundMap[pair.first].second->Stop();
					mSoundMap[pair.first].second = nullptr;
				}
			}


		}
		// 다른 플레이어의 경우 
		else {
			auto distanceSq = SimpleMath::Vector3::DistanceSquared(mMyPlayer->GetTransform().GetPosition(), pair.second->GetTransform().GetPosition());
			if (distanceSq > footprintDistance * footprintDistance) {
				continue;
			}

			// 움직인다면 
			if (pair.second->GetTransform().GetMovingState()) {
				// 이전에 멈춘 상태였다면
				if (mSoundMap[pair.first].second == nullptr) {
					mSoundMap[pair.first].first = mLayerIndexMap.GetLayerIndexAtPosition(pair.second->GetTransform().GetPosition());

					float volume = 1.0f - std::clamp(std::sqrtf(distanceSq) / footprintDistance, 0.0f, 1.0f);

					switch (mSoundMap[pair.first].first) {
					case 0: // Grass Area 
						mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("GrassArea", PrimaryVolume * volume, SoundOption::Shuffle);
						break;
					case 1: // Load Area
					case 2: // Stone Area 
						mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("DirtArea", PrimaryVolume * volume, SoundOption::Shuffle);
						break;
					default:
						break;
					}

				}
				else { // 이전에 움직이고 있던 상태였다면
					UINT currentAreaID{ mLayerIndexMap.GetLayerIndexAtPosition(pair.second->GetTransform().GetPosition()) };
					float volume = 1.0f - std::clamp(std::sqrtf(distanceSq) / footprintDistance, 0.0f, 1.0f);

					mSoundMap[pair.first].second->SetVolume(PrimaryVolume * volume);

					if (currentAreaID != mSoundMap[pair.first].first) { // Area가 바뀌었다면 

						if (mSoundMap[pair.first].second == nullptr) {
							return;
						}

						// 먼저 이전 Area 사운드를 빼야 한다. 
						mSoundMap[pair.first].second->Stop();

						mSoundMap[pair.first].first = currentAreaID;

						switch (mSoundMap[pair.first].first) {
						case 0: // Grass Area 
							mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("GrassArea", PrimaryVolume * volume, SoundOption::Shuffle);
							break;
						case 1: // Load Area
						case 2: // Stone Area 
							mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("DirtArea", PrimaryVolume * volume, SoundOption::Shuffle);
							break;
						default:
							break;
						}
					}

				}
			}
			else { // 멈춘 상태라면 
				if (mSoundMap[pair.first].second != nullptr) {
					mSoundMap[pair.first].second->Stop();
					mSoundMap[pair.first].second = nullptr;
				}
			}
		}
	}

	// 2. 발자국 소리 ( 오브젝트 ) 
	for (auto& pair : mGameObjectMap) {
		auto distanceSq = SimpleMath::Vector3::DistanceSquared(mMyPlayer->GetTransform().GetPosition(), pair.second->GetTransform().GetPosition());
		if (distanceSq > footprintDistance * footprintDistance) {
			continue;
		}

		// 움직인다면 
		if (pair.second->GetTransform().GetMovingState()) {
			// 이전에 멈춘 상태였다면
			if (mSoundMap[pair.first].second == nullptr) {
				mSoundMap[pair.first].first = mLayerIndexMap.GetLayerIndexAtPosition(pair.second->GetTransform().GetPosition());

				float volume = 1.0f - std::clamp(std::sqrtf(distanceSq) / footprintDistance, 0.0f, 1.0f);

				switch (mSoundMap[pair.first].first) {
				case 0: // Grass Area 
					mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("GrassAreaWalk", PrimaryVolume * volume, SoundOption::Shuffle);
					break;
				case 1: // Load Area
				case 2: // Stone Area 
					mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("DirtAreaWalk", PrimaryVolume * volume, SoundOption::Shuffle);
					break;
				default:
					break;
				}

			}
			else { // 이전에 움직이고 있던 상태였다면
				UINT currentAreaID{ mLayerIndexMap.GetLayerIndexAtPosition(pair.second->GetTransform().GetPosition()) };
				float volume = 1.0f - std::clamp(std::sqrtf(distanceSq) / footprintDistance, 0.0f, 1.0f);

				mSoundMap[pair.first].second->SetVolume(PrimaryVolume * volume);

				if (currentAreaID != mSoundMap[pair.first].first) { // Area가 바뀌었다면 
					// 먼저 이전 Area 사운드를 빼야 한다. 
					mSoundMap[pair.first].second->Stop();

					mSoundMap[pair.first].first = currentAreaID;

					switch (mSoundMap[pair.first].first) {
					case 0: // Grass Area 
						mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("GrassAreaWalk", PrimaryVolume * volume, SoundOption::Shuffle);
						break;
					case 1: // Load Area
					case 2: // Stone Area 
						mSoundMap[pair.first].second = SoundManager::GetInstance().PlaySound("DirtAreaWalk", PrimaryVolume * volume, SoundOption::Shuffle);
						break;
					default:
						break;
					}
				}

			}
		}
		else { // 멈춘 상태라면 
			if (mSoundMap[pair.first].second != nullptr) {
				mSoundMap[pair.first].second->Stop();
				mSoundMap[pair.first].second = nullptr;
			}
		}
	}
}

void ArenaScene::ProcessPackets(const uint8_t* buffer, size_t size) {
	const uint8_t* iter = buffer;

	while (iter < buffer + size) {
		iter = ProcessPacket(iter);
	}
 
}

const uint8_t* ArenaScene::ProcessPacket(const uint8_t* buffer) {

	decltype(auto) header = FbsPacketFactory::GetHeaderPtrSC(buffer);

	switch (header->type) {
	case Packets::PacketTypes_PT_OBJECT_REMOVED_SC:
	{
		ArenaScene::ProcessObjectRemoved(buffer);
	}
	break;
	case Packets::PacketTypes_PT_PLAYER_EXIT_SC:
	{
		ArenaScene::ProcessPlayerExit(buffer);
	}
	break;
	case Packets::PacketTypes_PT_LATENCT_SC:
	{
		ArenaScene::ProcessLatency(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_APPEARED_SC:
	{
		ArenaScene::ProcessObjectAppeared(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_DISAPPEARED_SC:
	{
		ArenaScene::ProcessObjectDisappeared(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_MOVE_SC:
	{
		ArenaScene::ProcessObjectMove(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_ATTACKED_SC:
	{
		ArenaScene::ProcessObjectAttacked(buffer);
	}
	break;
	case Packets::PacketTypes_PT_OBJECT_ANIMATION_CHANGED_SC:
	{
		ArenaScene::ProcessPacketAnimation(buffer);
	}
	break;
	case Packets::PacketTypes_PT_FIRE_PROJECTILE_SC:
	{
		ArenaScene::ProcessFireProjectile(buffer);
	}
	break;
	case Packets::PacketTypes_PT_PROJECTILE_MOVE_SC:
	{
		ArenaScene::ProcessProjectileMove(buffer);
	}
	break;
	case Packets::PacketTypes_PT_HEART_BEAT_SC:
	{
		ArenaScene::ProcessHeartBeat(buffer);
	}
	break;
	default:
		break;
	}

	return buffer + header->size;
}

void ArenaScene::Update() {
#ifdef DEV_MODE
	mLatencyBlock->GetText() = std::format(L"Latency : {} ms", ArenaScene::GetAverageLatency<std::chrono::milliseconds>());
#endif 
	float coefficient{ mIsBlind ? -1.f : 1.f };
	mRenderManager->GetFogRangeStart() += coefficient * Time.GetDeltaTime<float, std::chrono::seconds>() * 500.f;
	mRenderManager->GetFogRangeStart() = std::clamp(mRenderManager->GetFogRangeStart(), 7.f, 2000.f);


	mRenderManager->GetParticleManager().UpdateEmitParticle();

	if (mCurrentCameraMode) {
		mCurrentCameraMode->Update();

		/*auto& pos = mCamera.GetTransform().GetPosition();
		auto y = 0.f; 
		if (pos.y <= y + 0.5f) {
			pos.y = y + 0.5f;
		}*/

		mCurrentCameraMode->FocusUpdate();
	}
	mCamera.UpdateBuffer();
	mRenderManager->GetShadowRenderer().Update();

	static BoneTransformBuffer boneTransformBuffer{};
	for (auto& gameObject : mGameObjects | std::views::filter([](const GameObject& object) { return object.GetActiveState(); })) {
		if (gameObject.mAnimated) {
			gameObject.ForwardUpdate();
			gameObject.GetTransform().GetPosition().y = 0.f;

			gameObject.UpdateShaderVariables(boneTransformBuffer);

			auto [mesh, shader, modelContext] = gameObject.GetAnimationRenderData();
			float dissolveOffset = gameObject.GetDissolveOffset();
			if (mCamera.IsInFrustum(gameObject.mCollider)) {
				mRenderManager->GetMeshRenderManager().AppendBonedMeshContext(shader, mesh, modelContext, boneTransformBuffer, dissolveOffset);
			}

			// TODO :: 아예 의미가 없는 코드이다. 정석적인 CasCade 구현에서 벗어남. 
			/*for (UINT i = 0; i < Config::SHADOWMAP_COUNT<int>; ++i) {
				if (mRenderManager->GetShadowRenderer().ShadowMapCulling(i, gameObject.mCollider)) {
					mRenderManager->GetMeshRenderManager().AppendShadowBonedMeshContext(shader, mesh, modelContext, boneTransformBuffer, i);

				}
			}*/
		}
		else {
			gameObject.UpdateShaderVariables();

			auto [mesh, shader, modelContext] = gameObject.GetRenderData();

			mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(shader, mesh, modelContext);
			mRenderManager->GetMeshRenderManager().AppendShadowPlaneMeshContext(shader, mesh, modelContext, 0);
			mRenderManager->GetMeshRenderManager().AppendShadowPlaneMeshContext(shader, mesh, modelContext, 1);

		}

	}

	for (auto& player : mPlayers | std::views::filter([](const Player& p) { return p.GetActiveState(); })) {
		player.ForwardUpdate();
		player.GetTransform().GetPosition().y = 0.f;
		player.Update(mRenderManager->GetMeshRenderManager());
	}

	for (auto& object : mEnvironmentObjects) {
		object.UpdateLODLevel(mCamera.GetTransform().GetPosition());
		auto [mesh, shader, modelContext] = object.GetRenderData();

		if (mesh == nullptr) {
			continue;
		}

		if (object.mCollider.GetActiveState()) {
			for (int i = 0; i < Config::SHADOWMAP_COUNT<int>; ++i) {
				if (mRenderManager->GetShadowRenderer().IsInShadowFrustum(i, object.mCollider)) {
					mRenderManager->GetMeshRenderManager().AppendShadowPlaneMeshContext(shader, mesh, modelContext, i);
				}
			}

			if (mCamera.IsInFrustum(object.mCollider)) {
				mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(shader, mesh, modelContext);
			}
		} 
	}

	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition();

	mSkyBox.UpdateShaderVariables();
	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);

	ArenaScene::UpdateSound();
}

void ArenaScene::SendNetwork() {
	auto id = gClientCore2->GetSessionId();
	auto& keyTracker = Input.GetKeyboardTracker();

	for (const auto& key : std::views::iota(static_cast<uint8_t>(0), static_cast<uint8_t>(255))) {
		if (keyTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key)) or keyTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {
			if (keyTracker.IsKeyPressed(static_cast<DirectX::Keyboard::Keys>(key))) {
				decltype(auto) packet = FbsPacketFactory::PlayerInputCS(id, key, true);
				gClientCore2->Send(packet);
			}
			else if (keyTracker.IsKeyReleased(static_cast<DirectX::Keyboard::Keys>(key))) {
				decltype(auto) packet = FbsPacketFactory::PlayerInputCS(id, key, false);
				gClientCore2->Send(packet);
			}
		}
	}


	auto& mouseTracker = Input.GetMouseTracker();

	if (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED) {

		if (mMyPlayer != nullptr) {
			auto dir = mMyPlayer->GetTransform().GetForward();
			dir.y = 0.f;
			decltype(auto) packet = FbsPacketFactory::RequestAttackCS(gClientCore2->GetSessionId(), dir);
			gClientCore2->Send(packet);
		}
	}
}

void ArenaScene::Exit() {
	Input.EraseCallBack(mInputSign);
	mLatencyBlock->SetActiveState(false); 

	mExpired = true; 

	SoundManager::GetInstance().Reset();
}

void ArenaScene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::ifstream file("Resources/MeshList/ArenaSceneMeshList.txt");
	std::string line;

	MeshLoader loader{};

	const static std::unordered_map<std::string_view, EmbeddedMeshType> embeddedMeshMap{
		{ "Cube", EmbeddedMeshType::Cube },
		{ "SkyDome", EmbeddedMeshType::SkyDome },
		{ "Plane", EmbeddedMeshType::Plane }
	};

	while (std::getline(file, line)) {
		if (line.empty() || line.starts_with('#')) {
			continue;
		}

		std::istringstream iss(line);
		std::string type;
		iss >> type;

		if (type == "FILE") {
			std::string path, indexStr, name;
			iss >> path >> indexStr >> name;

			int index = (indexStr == "-") ? -1 : std::stoi(indexStr);
			MeshData data = (index == -1) ? loader.Load(path) : loader.Load(path, index);

			mMeshMap[name] = std::make_unique<Mesh>(device, commandList, data);
			mColliderMap[name] = Collider{ data.position };

			for (std::string token; iss >> token;) {
				if (token == "CENTER") {
					float x, y, z;
					iss >> x >> y >> z;
					mColliderMap[name].SetCenter(x, y, z);
				}
				else if (token == "EXTENTS") {
					float x, y, z;
					iss >> x >> y >> z;
					mColliderMap[name].SetExtents(x, y, z);
				}
			}
		}
		else if (type == "EMBEDDED") {
			std::string embeddedTypeStr, dash, name;
			float param;
			iss >> embeddedTypeStr >> dash >> name >> param;

			auto it = embeddedMeshMap.find(embeddedTypeStr);
			if (it == embeddedMeshMap.end()) {
				std::cerr << "Unknown embedded type: " << embeddedTypeStr << '\n';
				continue;
			}

			mMeshMap[name] = std::make_unique<Mesh>(device, commandList, it->second, param);
			
		}
	}
}

void ArenaScene::BuildMaterial() {
	mMaterialLoader = MaterialFileLoader{ mRenderManager };
	mMaterialLoader.Load();
}

void ArenaScene::BuildShader(ComPtr<ID3D12Device> device) {
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

	shader = std::make_unique<StandardNormalShader>();
	shader->CreateShader(device);
	mShaderMap["StandardNormalShader"] = std::move(shader);

	shader = std::make_unique<SkinnedNormalShader>();
	shader->CreateShader(device);
	mShaderMap["SkinnedNormalShader"] = std::move(shader);

	shader = std::make_unique<TreeCrossShader>();
	shader->CreateShader(device);
	mShaderMap["TreeCrossShader"] = std::move(shader);

	shader = std::make_unique<ArenaGroundShader>();
	shader->CreateShader(device);
	mShaderMap["ArenaGroundShader"] = std::move(shader);
}

void ArenaScene::BuildAniamtionController() {
	ArenaScene::BuildBaseAnimationController();

	ArenaScene::BuildSwordManAnimationController();
	ArenaScene::BuildArcherAnimationController();
	ArenaScene::BuildMageAnimationController();
	ArenaScene::BuildShieldManController();

	ArenaScene::BuildMonsterType1AnimationController();
	ArenaScene::BuildDemonAnimationController();
}

void ArenaScene::BuildEnvironment(const std::filesystem::path& envFile) {
	std::unordered_map<std::string, LODGameObject> objects{};

	std::ifstream file("Resources/Scene/ArenaSceneEnvPrefabs.txt");
	std::string line;

	while (std::getline(file, line)) {
		if (line.empty() || line.starts_with('#'))
			continue;

		std::istringstream iss(line);
		std::string objectName, colliderName;
		iss >> objectName >> colliderName;

		auto& obj = objects[objectName];
		obj.mCollider = mColliderMap[colliderName];
		obj.SetActiveState(true);
		obj.SetEmpty(false);

		std::getline(file, line);
		if (line.empty() || line.starts_with('#'))
			continue;

		int lodCount = std::stoi(line);
		for (int i = 0; i < lodCount; ++i) {
			std::getline(file, line);
			std::istringstream lodIss(line);

			int lodIndex;
			std::string meshName, lodShaderName, lodMaterialName;
			float distance;

			lodIss >> lodIndex >> meshName >> lodShaderName >> lodMaterialName >> distance;

			auto& lodGroup = obj.mLODGroups[lodIndex];
			lodGroup.mMesh = mMeshMap[meshName].get();
			lodGroup.mShader = mShaderMap[lodShaderName].get();
			lodGroup.mMaterial = mRenderManager->GetMaterialManager().GetMaterial(lodMaterialName);
			lodGroup.mDistanceSquared = std::powf(distance, 2.f);
		}
	}

	mEnvironmentObjects.reserve(1000);

	std::ifstream efile{ envFile, std::ios::binary };

	UINT objectCount;
	efile.read(reinterpret_cast<char*>(&objectCount), sizeof(UINT));

	struct Data {
		GameProtocol::EnvironmentType1 type;
		SimpleMath::Vector2 xzPosition;
		float yaw;
	};


	std::vector<Data> envData{};
	envData.resize(objectCount);

	efile.read(reinterpret_cast<char*>(envData.data()), sizeof(Data) * objectCount);

	for (auto& data : envData) {
		switch (data.type) {
			case GameProtocol::EnvironmentType1::Tower:
			{
				auto& obj = mEnvironmentObjects.emplace_back(objects["Tower"].Clone());
				obj.GetTransform().GetPosition() = { data.xzPosition.x, 0.f, data.xzPosition.y };
				obj.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(data.yaw), 0.f);
			}
			break;
			case GameProtocol::EnvironmentType1::Wall:
			{
				auto& obj = mEnvironmentObjects.emplace_back(objects["Wall"].Clone());
				obj.GetTransform().GetPosition() = { data.xzPosition.x, 0.f, data.xzPosition.y };
				obj.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(data.yaw), 0.f);
			}
			break;
			case GameProtocol::EnvironmentType1::Wall1:
			{
				auto& obj = mEnvironmentObjects.emplace_back(objects["Wall1"].Clone());
				obj.GetTransform().GetPosition() = { data.xzPosition.x, 0.f, data.xzPosition.y };
				obj.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(data.yaw), 0.f);
			}
			break;
			case GameProtocol::EnvironmentType1::DoorL:
			{
				auto& obj = mEnvironmentObjects.emplace_back(objects["DoorL"].Clone());
				obj.GetTransform().GetPosition() = { data.xzPosition.x, 0.f, data.xzPosition.y };
				obj.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(data.yaw), 0.f);
			}
			break;
			case GameProtocol::EnvironmentType1::DoorR:
			{
				auto& obj = mEnvironmentObjects.emplace_back(objects["DoorR"].Clone());
				obj.GetTransform().GetPosition() = { data.xzPosition.x, 0.f, data.xzPosition.y };
				obj.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(data.yaw), 0.f);
			}
			break;
			default:
				break; 
		}
	}

	auto& obj = mEnvironmentObjects.emplace_back(objects["Ground"].Clone());
	obj.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };

}

void ArenaScene::BuildBaseAnimationController() {
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

void ArenaScene::BuildArcherAnimationController() {
	{
		mAnimationMap["Archer"].Load("Resources/Assets/Knight/Archer/Archer.glb");
		auto& loader = mAnimationMap["Archer"];

		loader.AddClip(AppendAnimationClips(loader.GetClip(7), loader.GetClip(8)));


		std::vector<const AnimationClip*> clips{
			loader.GetClip(0), // Idle
			loader.GetClip(1), // Forward Run
			loader.GetClip(2), // Backward Run
			loader.GetClip(3), // Right Run
			loader.GetClip(4), // Left Run
			loader.GetClip(5), // Jump 
			loader.GetClip(6), // Attacked
			loader.GetClip(11), // Attack
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
		attackState.maskedClipIndex = 7;
		attackState.nonMaskedClipIndex = 7;
		attackState.name = "Attack";
		attackState.loop = false;

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
		mArcherAnimationController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
	}
}

void ArenaScene::BuildSwordManAnimationController() {
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
		attackState.maskedClipIndex = 7;
		attackState.nonMaskedClipIndex = 7;
		attackState.name = "Attack";
		attackState.loop = true;

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

void ArenaScene::BuildMageAnimationController() {
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
		attackState.maskedClipIndex = 7;
		attackState.nonMaskedClipIndex = 7;
		attackState.name = "Attack";
		attackState.loop = true;

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

void ArenaScene::BuildShieldManController() {
	mAnimationMap["ShieldMan"].Load("Resources/Assets/Knight/ShieldMan/ShieldMan.glb");
	auto& loader = mAnimationMap["ShieldMan"];

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
	attackState.maskedClipIndex = 7;
	attackState.nonMaskedClipIndex = 7;
	attackState.name = "Attack";
	attackState.loop = true;

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
	mShieldManController = AnimatorGraph::BoneMaskAnimationGraphController(clips, boneMask, states);
}

void ArenaScene::BuildMonsterType1AnimationController() {
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

	mMonsterAnimationController = AnimatorGraph::AnimationGraphController({ idleState, forwardState, backwardState, leftState, rightState, jumpState, attackedState, attackState, interactionState, deathState });
}

void ArenaScene::BuildDemonAnimationController() {
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
	attackState.loop = true;

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

