#include "pch.h"
#include "LoadingScene.h"
#include "../Game/System/Timer.h"

LoadingScene::LoadingScene(std::shared_ptr<RenderManager> renderMgr) {
	mLoading.Init(renderMgr->GetCanvas(), renderMgr->GetTextureManager().GetTexture("_Run"), 10, 1, 0.7f);

	mLoading.GetRect().width = static_cast<float>(400);
	mLoading.GetRect().height = static_cast<float>(400);
	mLoading.GetRect().LTx = -150.f;
	mLoading.GetRect().LTy = static_cast<float>(Config::WINDOW_HEIGHT<float> -mLoading.GetRect().height - 160.f);


	mBackground.Init(renderMgr->GetCanvas(), renderMgr->GetTextureManager().GetTexture("bg"));
	mLoadingWord.Init(renderMgr->GetCanvas(), renderMgr->GetTextureManager().GetTexture("loading"));
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

void LoadingScene::Exit() {

}
