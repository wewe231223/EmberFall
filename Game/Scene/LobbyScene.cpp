#include "pch.h"
#include "LobbyScene.h"

LobbyScene::LobbyScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation) {
	mRenderManager = renderMgr;

	mCamera = Camera(mainCamLocation);
	auto& cameraTransform = mCamera.GetTransform();
	cameraTransform.GetPosition() = { 100.f, 100.f, 100.f };
	cameraTransform.Look({ 0.f,85.f,0.f });
}

LobbyScene::~LobbyScene() {
}

void LobbyScene::Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	LobbyScene::BuildShader(device);
	LobbyScene::BuildMesh(device, commandList);
	LobbyScene::BuildMaterial();

	mSkyBox.mShader = mShaderMap["SkyBoxShader"].get();
	mSkyBox.mMesh = mMeshMap["SkyBox"].get();
	mSkyBox.mMaterial = mRenderManager->GetMaterialManager().GetMaterial("SkyBoxMaterial");
}

void LobbyScene::ProcessNetwork() {
}

void LobbyScene::Update() {
	mCamera.UpdateBuffer(); 

	mSkyBox.GetTransform().GetPosition() = mCamera.GetTransform().GetPosition(); 
	mSkyBox.UpdateShaderVariables();

	mSkyBox.UpdateShaderVariables();
	auto [skyBoxMesh, skyBoxShader, skyBoxModelContext] = mSkyBox.GetRenderData();
	mRenderManager->GetMeshRenderManager().AppendPlaneMeshContext(skyBoxShader, skyBoxMesh, skyBoxModelContext, 0);
}

void LobbyScene::SendNetwork() {
}

void LobbyScene::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mMeshMap["SkyBox"] = std::make_unique<Mesh>(device, commandList, 100.f);
}

void LobbyScene::BuildMaterial() {
	MaterialConstants mat{};

	mat.mDiffuseTexture[0] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Front_0");
	mat.mDiffuseTexture[1] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Back_0");
	mat.mDiffuseTexture[2] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Top_0");
	mat.mDiffuseTexture[3] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Bottom_0");
	mat.mDiffuseTexture[4] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Left_0");
	mat.mDiffuseTexture[5] = mRenderManager->GetTextureManager().GetTexture("SkyBox_Right_0");
	mRenderManager->GetMaterialManager().CreateMaterial("SkyBoxMaterial", mat);
}


void LobbyScene::BuildShader(ComPtr<ID3D12Device> device) {
	std::unique_ptr<GraphicsShaderBase> shader = std::make_unique<SkyBoxShader>();
	shader->CreateShader(device);
	mShaderMap["SkyBoxShader"] = std::move(shader);


}
