#include "pch.h"
#include "Scene.h"
#include "../Renderer/Core/Renderer.h"
#include "../MeshLoader/Loader/MeshLoader.h"
#ifdef _DEBUG
#pragma comment(lib,"out/debug/MeshLoader.lib")
#else 
#pragma comment(lib,"out/release/MeshLoader.lib")
#endif

Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers) {
	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);

	MeshLoader loader{};
	auto meshData = loader.Load("Resources/Assets/monster.gltf");

	mMeshMap["Cube"] = std::make_unique<PlainMesh>(device, commandList, EmbeddedMeshType::Sphere, 5);
	mMeshMap["T_Pose"] = std::make_unique<PlainMesh>(device, commandList, meshData);


	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);

	mShaderMap["StandardShader"] = std::move(shader);

	MaterialConstants material{};
	material.mDiffuseColor = { 1.f, 0.f, 1.0f, 1.0f };
	material.mDiffuseTexture[0] = mTextureManager->GetTexture("Creep_BaseColor");

	mMaterialManager->CreateMaterial("CubeMaterial", material);

	auto& object = mGameObjects.emplace_back();
	object.mShader = mShaderMap["StandardShader"].get();
	object.mMesh = mMeshMap["T_Pose"].get();
	object.mMaterial = mMaterialManager->GetMaterial("CubeMaterial");
	object.GetTransform().Scaling(100.f, 100.f, 100.f);

	mCamera = Camera(device);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,0.f,0.f });

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
