#include "pch.h"
#include "Scene.h"
#include "../Renderer/Core/Renderer.h"


Scene::Scene(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> managers) {
	mMeshRenderManager = std::get<0>(managers);
	mTextureManager = std::get<1>(managers);
	mMaterialManager = std::get<2>(managers);

	auto& object = mGameObjects.emplace_back();

	mMeshMap["Cube"] = std::make_unique<PlainMesh>(device, commandList, EmbeddedMeshType::Sphere, 1);

	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<StandardShader>();
	shader->CreateShader(device);

	mShaderMap["StandardShader"] = std::move(shader);

	MaterialConstants material{};
	material.mDiffuseColor = { 1.f, 0.f, 1.0f, 1.0f };

	mMaterialManager->CreateMaterial("CubeMaterial", material);

	object.mShader = mShaderMap["StandardShader"].get();
	object.mMesh = mMeshMap["Cube"].get();
	object.mMaterial = mMaterialManager->GetMaterial("CubeMaterial");

	auto& transform = object.GetTransform();
	transform.GetPosition() = { 0.f, 0.f, 0.f };
	transform.GetScale() = { 1.f, 1.f, 1.f };


	mCamera = Camera(device);
	
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,0.f,0.f });
}

void Scene::Update() {
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
