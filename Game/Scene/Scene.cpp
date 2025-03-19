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
#include "../Game/System/Timer.h"
#include "../Game/System/Input.h"
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
		//glfwSetWindowShouldClose()

		MessageBox(nullptr, L"ERROR!!!!!\nProtocolVersion Mismatching", L"", MB_OK | MB_ICONERROR);
		::exit(0);
	}
}

void Scene::ProcessPlayerPacket(PacketHeader* header) {

}

void Scene::ProcessObjectPacket(PacketHeader* header) {

}

void Scene::ProcessObjectDead(PacketHeader* header) {

}

void Scene::ProcessObjectAppeared(PacketHeader* header) {

}

void Scene::ProcessObjectDisappeared(PacketHeader* header) {

}

void Scene::ProcessPlayerExit(PacketHeader* header) {

}

void Scene::ProcessAcquiredItem(PacketHeader* header) {

}

void Scene::ProcessObjectAttacked(PacketHeader* header) {

}

void Scene::ProcessUseItem(PacketHeader* header) {

}

void Scene::ProcessRestoreHP(PacketHeader* header) {

}
#pragma endregion 


Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation) {
	
	mInputSign = NonReplacementSampler::GetInstance().Sample(); 
	
	gClientCore->Init();
	if (false == gClientCore->Start("127.0.0.1", 7777)) {
		Crash(false);
	}

	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);

	Scene::BuildShader(device); 
	Scene::BuildMesh(device, commandList);
	Scene::BuildMaterial();
	Scene::BuildPacketProcessor();

	tCollider.LoadFromFile("Resources/Binarys/Terrain/TerrainBaked.bin");

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mMaterialManager->GetMaterial("SkyBoxMaterial");


	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["TerrainShader"].get();
		object.mMesh = mMeshMap["Terrain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("TerrainMaterial");
		
		object.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };
		object.GetTransform().Scaling(1.f, 1.f, 1.f);
	}


	{
		auto& object = mGameObjects.emplace_back(); 
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["Mountain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("MountainMaterial");

		object.GetTransform().GetPosition() = { 370.f,tCollider.GetHeight(370.f, 300.f) - 10.f ,300.f };
		object.GetTransform().Rotate(0.f, DirectX::XMConvertToRadians(-35.f), 0.f);
	}

	mAnimationMap["Man"].Load("Resources/Assets/Knight/archer.gltf");
	Scene::BuildAniamtionController(); 


	mHumanPlayers[0] = Player(mMeshMap["T_Pose"].get(), mShaderMap["SkinnedShader"].get(), mMaterialManager->GetMaterial("CubeMaterial"), mArcherAnimationController);

	mMyPlayer = mHumanPlayers.begin();
	Scene::SetInputArcherMode(); 

		//AnimatorGraph::AnimationState idleState{}; 
		//idleState.stateIndex = 0;
		//idleState.name = "Idle";
		//idleState.clip = mAnimationMap["Man"].GetClip(0);
		//{
		//	AnimatorGraph::AnimationTransition idleToRun{};
		//	idleToRun.targetStateIndex = 1;
		//	idleToRun.blendDuration = 0.09;
		//	idleToRun.parameterName = "Speed";
		//	idleToRun.expectedValue = true;
		//	idleState.transitions.emplace_back(idleToRun);
		//}


		//AnimatorGraph::AnimationState stateRun;
		//stateRun.stateIndex = 1;
		//stateRun.name = "Run";
		//stateRun.clip = mAnimationMap["Man"].GetClip(3);
		//{
		//	AnimatorGraph::AnimationTransition runToIdle;
		//	runToIdle.targetStateIndex = 0;
		//	runToIdle.blendDuration = 0.09;
		//	runToIdle.parameterName = "Speed";
		//	runToIdle.expectedValue = false;
		//	stateRun.transitions.emplace_back(runToIdle);
		//}

		//std::vector<AnimatorGraph::AnimationState> states{ idleState, stateRun };


		//mPlayer.mGraphController = AnimatorGraph::AnimationGraphController(states);
		//mPlayer.mGraphController.AddParameter("Speed", AnimatorGraph::ParameterType::Bool);





	//	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, sign, [this]() {
	//		mPlayer.GetTransform().Translate({ 0.f, 0.f, -speed });
	//		mPlayer.mGraphController.SetBool("Speed", true);
	//		mPlayer.mBoneMaskGraphController.SetBool("Run", true);
	//		});

	//	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::W, sign, [this]() {
	//		mPlayer.mGraphController.SetBool("Speed", false);
	//		mPlayer.mBoneMaskGraphController.SetBool("Run", false);
	//		});

	//	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::S, sign, [this]() {
	//		mPlayer.GetTransform().Translate({ 0.f, 0.f, speed });
	//		});

	//	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, sign, [this]() {
	//		mPlayer.GetTransform().Translate({ speed, 0.f, 0.f });
	//		});

	//	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, sign, [this]() {
	//		mPlayer.GetTransform().Translate({ -speed, 0.f, 0.f });
	//		});
	//

	//	Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F, sign, [this]() {
	//		mPlayer.mBoneMaskGraphController.SetTrigger("Attack");
	//		speed = 0.005f;
	//		});

	//	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::F, sign, [this]() { speed = 0.01f; });
	//}






	mCamera = Camera(mainCameraBufferLocation);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });

	 mCameraMode = std::make_unique<TPPCameraMode>(&mCamera, mMyPlayer->GetTransform(), SimpleMath::Vector3{0.f,2.5f,7.f});
	// mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);

	mCameraMode->Enter();


}

void Scene::ProcessNetwork() {
	auto packetHandler = gClientCore->GetPacketHandler(); 
	decltype(auto) buffer = packetHandler->GetBuffer(); 

	mPacketProcessor.ProcessPackets(buffer);
}

void Scene::Update() {
	mMyPlayer->GetTransform().GetPosition().y = tCollider.GetHeight(mMyPlayer->GetTransform().GetPosition().x, mMyPlayer->GetTransform().GetPosition().z);
	mCameraMode->Update();
	
	for (auto& gameObject : mGameObjects) {
		if (gameObject) {
			gameObject.UpdateShaderVariables();
			auto [mesh, shader, modelContext] = gameObject.GetRenderData();
			mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
		}
	}

	for (auto& player : mHumanPlayers) {
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

void Scene::BuildPacketProcessor() {
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_PROTOCOL_VERSION, [this](PacketHeader* header) { ProcessPacketProtocolVersion(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_NOTIFY_ID, [this](PacketHeader* header) { ProcessNotifyId(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_PLAYER, [this](PacketHeader* header) { ProcessPlayerPacket(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT, [this](PacketHeader* header) { ProcessObjectPacket(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT_APPEARED, [this](PacketHeader* header) { ProcessObjectAppeared(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT_DISAPPEARED, [this](PacketHeader* header) { ProcessObjectDisappeared(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_OBJECT_DEAD, [this](PacketHeader* header) { ProcessObjectDead(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_RESTORE_HEALTH, [this](PacketHeader* header) { ProcessRestoreHP(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_USE_ITEM, [this](PacketHeader* header) { ProcessUseItem(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_PLAYER_EXIT, [this](PacketHeader* header) { ProcessPlayerExit(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PACKET_ATTACKED, [this](PacketHeader* header) { ProcessObjectAttacked(header); });
	mPacketProcessor.RegisterProcessFn(PacketType::PAKCET_ACQUIRED_ITEM, [this](PacketHeader* header) { ProcessAcquiredItem(header); });
}

void Scene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	MeshLoader Loader{};
	MeshData data{}; 

	data = Loader.Load("Resources/Assets/Tree/stem/Ash1.gltf");
	mMeshMap["Tree_Stem"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Tree/leaves1/Ash1.gltf");
	mMeshMap["Tree_Leaves"] = std::make_unique<Mesh>(device, commandList, data);

	mMeshMap["Cube"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Cube, 1);

	data = Loader.Load("Resources/Assets/Knight/archer.gltf");
	mMeshMap["T_Pose"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone1.gltf");
	mMeshMap["Stone1"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone2.gltf");
	mMeshMap["Stone2"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone3.gltf");
	mMeshMap["Stone3"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Mountain/Mountain.gltf");
	mMeshMap["Mountain"] = std::make_unique<Mesh>(device, commandList, data);
	
	data = Loader.Load("Resources/Assets/Weapon/sword/sword.gltf");
	mMeshMap["Sword"] = std::make_unique<Mesh>(device, commandList, data);

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
	mat.mDiffuseTexture[1] = mTextureManager->GetTexture("Detail_Texture_7");
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

	mat.mDiffuseTexture[0] = mTextureManager->GetTexture("M_LongSword_baseColor");
	mMaterialManager->CreateMaterial("SwordMaterial", mat);
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

	// Archer 
	{
		std::vector<const AnimationClip*> clips{ mAnimationMap["Man"].GetClip(0), mAnimationMap["Man"].GetClip(1), mAnimationMap["Man"].GetClip(16), mAnimationMap["Man"].GetClip(3) };

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

void Scene::SetInputArcherMode() {

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetBool("Move", true);
		});

	Input.RegisterKeyReleaseCallBack(DirectX::Keyboard::Keys::W, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetBool("Move", false);
		});


	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, mInputSign, [this]() {
		});

	Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, mInputSign, [this]() {
		});
	

	Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::F, mInputSign, [this]() {
		mMyPlayer->GetBoneMaskController().SetTrigger("Attack");
		});
}

void Scene::SetInputSwordManMode() {

}

void Scene::SetInputMageMode() {

}






