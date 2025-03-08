#include "pch.h"
#include "Scene.h"
#include "../Renderer/Core/Renderer.h"
#include "../MeshLoader/Loader/MeshLoader.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#include "../MeshLoader/Loader/TerrainLoader.h"
#ifdef _DEBUG
#pragma comment(lib,"out/debug/MeshLoader.lib")
#else 
#pragma comment(lib,"out/release/MeshLoader.lib")
#endif

#include <ranges>
#include "../Game/System/Timer.h"

Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers, DefaultBufferCPUIterator mainCameraBufferLocation) {
	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);

	std::filesystem::path ZombiePath = "Resources/Assets/zombie/scene.gltf";
	std::filesystem::path CreepPath = "Resources/Assets/CreepMonster/Creep_mesh.gltf";
	std::filesystem::path ManPath = "Resources/Assets/Man/man.gltf";
	std::filesystem::path assetPath = ManPath;

	MeshLoader loader{};
	auto meshData = loader.Load(assetPath);


	mMeshMap["Cube"] = std::make_unique<Mesh>(device, commandList, EmbeddedMeshType::Sphere, 5);
	mMeshMap["T_Pose"] = std::make_unique<Mesh>(device, commandList, meshData);


	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);
	mShaderMap["StandardShader"] = std::move(shader);

	MaterialConstants material{};
	material.mDiffuseTexture[0] = mTextureManager->GetTexture("Base_Texture");
	material.mDiffuseTexture[1] = mTextureManager->GetTexture("Detail_Texture_7");
	mMaterialManager->CreateMaterial("TerrainMaterial", material);

	TerrainLoader terrainLoader{};
	auto terrainData = terrainLoader.Load("Resources/Binarys/Terrain/HeightMap.raw", true);
	mMeshMap["Terrain"] = std::make_unique<Mesh>(device, commandList, terrainData);

	shader = std::make_unique<TerrainShader>();
	shader->CreateShader(device);
	mShaderMap["TerrainShader"] = std::move(shader);

	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["TerrainShader"].get();
		object.mMesh = mMeshMap["Terrain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("TerrainMaterial");
		
		object.GetTransform().GetPosition() = { 0.f, 0.f, 0.f };
		object.GetTransform().Scaling(5.f, 1.f, 5.f);
	}


	MaterialConstants skyBoxMaterial{};
	skyBoxMaterial.mDiffuseTexture[0] = mTextureManager->GetTexture("SkyBox_Front_0");
	skyBoxMaterial.mDiffuseTexture[1] = mTextureManager->GetTexture("SkyBox_Back_0");
	skyBoxMaterial.mDiffuseTexture[2] = mTextureManager->GetTexture("SkyBox_Top_0");
	skyBoxMaterial.mDiffuseTexture[3] = mTextureManager->GetTexture("SkyBox_Bottom_0");
	skyBoxMaterial.mDiffuseTexture[4] = mTextureManager->GetTexture("SkyBox_Left_0");
	skyBoxMaterial.mDiffuseTexture[5] = mTextureManager->GetTexture("SkyBox_Right_0");

	mMaterialManager->CreateMaterial("SkyBoxMaterial", skyBoxMaterial);

	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 900.f);

	shader = std::make_unique<SkyBoxShader>();
	shader->CreateShader(device);

	mShaderMap["SkyBoxShader"] = std::move(shader);

	{
		mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
		mSkyBox.mMesh = mMeshMap["SkyBox"].get();
		mSkyBox.mMaterial = mMaterialManager->GetMaterial("SkyBoxMaterial");
	}


	material.mDiffuseTexture[0] = mTextureManager->GetTexture("Ganfaul_diffuse");
	mMaterialManager->CreateMaterial("CubeMaterial", material);


	shader = std::make_unique<SkinnedShader>();
	shader->CreateShader(device);
	mShaderMap["SkinnedShader"] = std::move(shader);

	AnimationLoader Animloader{};
	// auto animationData = Animloader.Load(assetPath,10);
	auto animationData = Animloader.Load(assetPath,2);

	testAnimator = Animator(animationData);

	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["SkinnedShader"].get();
		object.mMesh = mMeshMap["T_Pose"].get();
		object.mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
		object.GetTransform().Translate({ 0.f,85.f,0.f });
		object.GetTransform().Scaling(10.f, 10.f, 10.f);
	}





	mCamera = Camera(mainCameraBufferLocation);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });

	mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
	mCameraMode->Enter();
}

void Scene::Update() {
	mCameraMode->Update();
	
	for (auto& gameObject : mGameObjects | std::views::take(1)) {
		if (gameObject) {
			auto& transform = gameObject.GetTransform();
			transform.UpdateWorldMatrix();

			auto [mesh, shader, modelContext] = gameObject.GetRenderData();
			mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
		}
	}



	static double counter = 0.0;

	counter += 0.001;
	std::vector<SimpleMath::Matrix> boneTransforms{};
	testAnimator.UpdateBoneTransform(counter, boneTransforms);

	auto& object = mGameObjects.back();
	auto& transform = object.GetTransform();
	transform.UpdateWorldMatrix();

	auto [mesh, shader, modelContext] = object.GetRenderData();

	mMeshRenderManager->AppendBonedMeshContext(shader, mesh, modelContext, boneTransforms);

	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition();
	mSkyBox.UpdateShaderVariables();

	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mMeshRenderManager->AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);

	mCamera.UpdateBuffer(); 
}


