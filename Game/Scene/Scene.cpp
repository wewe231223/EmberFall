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
#include <ranges>
#include <random>
#include "../Game/System/Timer.h"
#include "../Game/System/Input.h"
#include "../Utility/NonReplacementSampler.h"
#include "../MeshLoader/Loader/TerrainBaker.h"

Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation) {

//	SimulateTessellationAndWriteFile("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", "Resources/Binarys/Terrain/TerrainBaked.bin");
	tCollider.LoadFromFile("Resources/Binarys/Terrain/TerrainBaked.bin");


	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);

	Scene::BuildShader(device); 
	Scene::BuildMesh(device, commandList);
	Scene::BuildMaterial();
	Scene::PaintTree(100); 


	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mMaterialManager->GetMaterial("SkyBoxMaterial");

	//{
	//	auto& object = mGameObjects.emplace_back();
	//	object.mShader = mShaderMap["StandardShader"].get();
	//	object.mMesh = mMeshMap["Cube"].get();
	//	object.mMaterial = mMaterialManager->GetMaterial("AreaMat");
	//	object.GetTransform().GetPosition() = { 0.f,0.f,0.f };
	//	object.GetTransform().Scaling(500.f, 500.f, 500.f);
	//}

	
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



	mAnimationMap["Man"].Load("Resources/Assets/Knight/Knight.gltf");
	auto animationData = mAnimationMap["Man"].GetClip(1);

	{
		mPlayer.mShader = mShaderMap["SkinnedShader"].get();
		mPlayer.mMesh = mMeshMap["T_Pose"].get();
		mPlayer.mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
		mPlayer.GetTransform().Translate({ 0.f,20.f,0.f });
		mPlayer.mAnimator = Animator(animationData);

		std::vector<const AnimationClip*> clips{ mAnimationMap["Man"].GetClip(29), mAnimationMap["Man"].GetClip(31) };
		mPlayer.mGraphAnimator = AnimatorGraph::Animator(clips);

		const size_t step = 0;
		int sign = NonReplacementSampler::GetInstance().Sample();

		Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::W, sign, [this]() {
			mPlayer.GetTransform().Translate({ 0.f, 0.f, -0.1f });
			});

		Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::S, sign, [this]() {
			mPlayer.GetTransform().Translate({ 0.f, 0.f, 0.1f });
			});

		Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::A, sign, [this]() {
			mPlayer.GetTransform().Translate({ 0.1f, 0.f, 0.f });
			});

		Input.RegisterKeyPressCallBack(DirectX::Keyboard::Keys::D, sign, [this]() {
			mPlayer.GetTransform().Translate({ -0.1f, 0.f, 0.f });
			});
	
		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D1, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D2, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 1);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D3, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 2);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D4, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 3);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D5, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 4);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D6, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 5);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D7, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 6);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D8, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 7);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D9, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 8);
			});

		Input.RegisterKeyDownCallBack(DirectX::Keyboard::Keys::D0, sign, [this]() {
			mPlayer.mGraphAnimator.TransitionToClip(step + 9);
			});




	}




	mCamera = Camera(mainCameraBufferLocation);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });

	mCameraMode = std::make_unique<TPPCameraMode>(&mCamera, mPlayer.GetTransform(), SimpleMath::Vector3{0.f,2.f,5.f});
	// mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);

	mCameraMode->Enter();

	mPickedObjectText->GetText() = L"Picked Object : None";
}

void Scene::Update() {
	mPlayer.GetTransform().GetPosition().y = tCollider.GetHeight(mPlayer.GetTransform().GetPosition().x, mPlayer.GetTransform().GetPosition().z);
	mCameraMode->Update();
	
	static BoneTransformBuffer boneTransforms{};


	for (auto& gameObject : mGameObjects) {
		if (gameObject) {
			gameObject.UpdateShaderVariables(boneTransforms);

			auto [mesh, shader, modelContext] = gameObject.GetRenderData();
			if (gameObject.GetAnimatorState()) {
				mMeshRenderManager->AppendBonedMeshContext(shader, mesh, modelContext, boneTransforms);
			}
			else {
				mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
			}
		}
	}

	mPlayer.UpdateShaderVariables(boneTransforms);
	auto [mesh, shader, modelContext] = mPlayer.GetRenderData();
	mMeshRenderManager->AppendBonedMeshContext(shader, mesh, modelContext, boneTransforms);


	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition();
	mSkyBox.UpdateShaderVariables(boneTransforms);

	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mMeshRenderManager->AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);

	mCamera.UpdateBuffer(); 
}

void Scene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	MeshLoader Loader{};
	MeshData data{}; 

	data = Loader.Load("Resources/Assets/Tree/stem/Ash1.gltf");
	mMeshMap["Tree_Stem"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Tree/leaves1/Ash1.gltf");
	mMeshMap["Tree_Leaves"] = std::make_unique<Mesh>(device, commandList, data);

	mMeshMap["Cube"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Cube, 1);

	data = Loader.Load("Resources/Assets/Knight/Knight.gltf");
	mMeshMap["T_Pose"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone1.gltf");
	mMeshMap["Stone1"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone2.gltf");
	mMeshMap["Stone2"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Stone/Stone3.gltf");
	mMeshMap["Stone3"] = std::make_unique<Mesh>(device, commandList, data);

	data = Loader.Load("Resources/Assets/Mountain/Mountain.gltf");
	mMeshMap["Mountain"] = std::make_unique<Mesh>(device, commandList, data);
	
	data = tLoader.Load("Resources/Binarys/Terrain/Rolling Hills Height Map.raw", true);
	// terrainLoader.ExportTessellatedMesh("Resources/Binarys/Terrain/TerrainBaked.bin", 16.f);

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

void Scene::PaintTree(size_t treeCount) {
	size_t mapSize = std::filesystem::file_size("Resources/Binarys/Terrain/Tree.raw");
	int dimension = static_cast<int>(std::sqrt(mapSize)); // 가로, 세로 길이

	std::unique_ptr<BYTE[]> treeMap{ std::make_unique<BYTE[]>(mapSize) };
	std::ifstream in{ "Resources/Binarys/Terrain/Tree.raw", std::ios::binary };
	in.read(reinterpret_cast<char*>(treeMap.get()), mapSize);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, dimension - 1);
	std::uniform_int_distribution<int> stonedist(0, 2);

	GameObject stone{}; 
	stone.mShader = mShaderMap["StandardShader"].get();
	stone.mMesh = mMeshMap["Stone1"].get();
	stone.mMaterial = mMaterialManager->GetMaterial("StoneMaterial");
	// stone.GetTransform().Scaling(10.f, 10.f, 10.f);

	while (mGameObjects.size() < treeCount) {
		int x = dist(gen);
		int z = dist(gen);

		if (treeMap[x + z * dimension] != 0) {
			float centeredX = static_cast<float>(x - dimension / 2);
			float centeredZ = static_cast<float>(z - dimension / 2);

			stone.GetTransform().GetPosition() = { centeredX, 20.f, centeredZ };
			stone.GetTransform().Scaling(30.f, 30.f, 30.f);

			switch (stonedist(gen)) {
			case 0:
				stone.mMesh = mMeshMap["Stone1"].get();
				break;
			case 1:
				stone.mMesh = mMeshMap["Stone2"].get();
				break;
			case 2:
				stone.mMesh = mMeshMap["Stone3"].get();
				break;
			}

			stone.GetTransform().GetPosition().y = tCollider.GetHeight(centeredX, centeredZ);

			mGameObjects.emplace_back(stone);

		}
	}

}


