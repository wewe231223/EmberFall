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

Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers) {
	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);

	std::filesystem::path ZombiePath = "Resources/Assets/zombie/scene.gltf";
	std::filesystem::path CreepPath = "Resources/Assets/CreepMonster/Creep_mesh.gltf";
	std::filesystem::path assetPath = CreepPath;

	MeshLoader loader{};
	auto meshData = loader.Load(assetPath);

	UINT max = 0;
	for (auto& ids : meshData.boneID) {
		UINT m{ 0 };
		m = *std::max_element(ids.begin(), ids.end());
		if (m > max) max = m;
	}

	std::vector<int>as(max);


	mMeshMap["Cube"] = std::make_unique<PlainMesh>(device, commandList, EmbeddedMeshType::Sphere, 5);
	mMeshMap["T_Pose"] = std::make_unique<PlainMesh>(device, commandList, meshData);


	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);
	mShaderMap["StandardShader"] = std::move(shader);

	MaterialConstants material{};
	material.mDiffuseTexture[0] = mTextureManager->GetTexture("Creep_BaseColor");

	mMaterialManager->CreateMaterial("CubeMaterial", material);


	material.mDiffuseTexture[0] = mTextureManager->GetTexture("Base_Texture");
	material.mDiffuseTexture[1] = mTextureManager->GetTexture("Detail_Texture_7");

	mMaterialManager->CreateMaterial("TerrainMaterial", material);

	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["StandardShader"].get();
		object.mMesh = mMeshMap["T_Pose"].get();
		object.mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
		object.GetTransform().Scaling(10000.f, 10000.f, 10000.f);
	}

	mCamera = Camera(device);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,0.f,0.f });


	TerrainLoader terrainLoader{};
	auto terrainData = terrainLoader.Load("Resources/Binarys/Terrain/HeightMap.raw", true);
	mMeshMap["Terrain"] = std::make_unique<PlainMesh>(device, commandList, terrainData);

	shader = std::make_unique<TerrainShader>();
	shader->CreateShader(device);
	mShaderMap["TerrainShader"] = std::move(shader);

	{
		auto& object = mGameObjects.emplace_back();
		object.mShader = mShaderMap["TerrainShader"].get();
		object.mMesh = mMeshMap["Terrain"].get();
		object.mMaterial = mMaterialManager->GetMaterial("TerrainMaterial");
		
		object.GetTransform().GetPosition() = { 100.f, 0.f, 100.f };
		object.GetTransform().Scaling(5.f, 1.f, 5.f);

	}


	AnimationLoader Animloader{};
	auto animationData = Animloader.Load(assetPath);


	mCameraMode = std::make_unique<FreeCameraMode>(&mCamera);
	mCameraMode->Enter();
}

void Scene::Update() {
	mCameraMode->Update();

	for (auto& gameObject : mGameObjects) {
		if (gameObject) {
			auto& transform = gameObject.GetTransform();
			transform.UpdateWorldMatrix();

			auto [mesh, shader, modelContext] = gameObject.GetRenderData();
			mMeshRenderManager->AppendPlaneMeshContext(shader, mesh, modelContext);
		}
	}
}

void Scene::PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mCamera.Bind(commandList);
}
