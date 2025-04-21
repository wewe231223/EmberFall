
#include "pch.h"
#include "LoadingScene.h"

LoadingScene::LoadingScene(std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>, std::shared_ptr<Canvas>> managers) {
	mLoading.Init(std::get<4>(managers), std::get<1>(managers)->GetTexture("circle 2"), 12, 1, 0.5f);

}

LoadingScene::~LoadingScene() {
}

void LoadingScene::Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) {

}

void LoadingScene::ProcessNetwork() {
}

void LoadingScene::Update() {
	mLoading.Update();
}

void LoadingScene::SendNetwork() {
}
