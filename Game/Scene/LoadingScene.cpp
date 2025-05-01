#include "pch.h"
#include "LoadingScene.h"
#include "../Game/System/Timer.h"

LoadingScene::LoadingScene(std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>, std::shared_ptr<Canvas>> managers) {
	mLoading.Init(std::get<4>(managers), std::get<1>(managers)->GetTexture("_Run"), 10, 1, 0.7f);

	mLoading.GetRect().width = static_cast<float>(400);
	mLoading.GetRect().height = static_cast<float>(400);
	mLoading.GetRect().LTx = -150.f;
	mLoading.GetRect().LTy = static_cast<float>(Config::WINDOW_HEIGHT<float> -mLoading.GetRect().height - 160.f);


	mBackground.Init(std::get<4>(managers), std::get<1>(managers)->GetTexture("bg"));
	mLoadingWord.Init(std::get<4>(managers), std::get<1>(managers)->GetTexture("loading"));
	mLoadingWord.GetRect() = { Config::WINDOW_WIDTH<float> / 2.f - 512.f, 100.f, 1024.f, 170.f };
}

LoadingScene::~LoadingScene() {
}

void LoadingScene::Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) {

}

void LoadingScene::ProcessNetwork() {
}

void LoadingScene::Update() {
	mLoading.GetRect().LTx += Time.GetDeltaTime<float>() * 200.f; 

	if (mLoading.GetRect().LTx > Config::WINDOW_WIDTH<float> + 150.f) {
		mLoading.GetRect().LTx = -200.f;
	}


	mBackground.Update();
	mLoadingWord.Update();
	mLoading.Update();
}

void LoadingScene::SendNetwork() {
}
