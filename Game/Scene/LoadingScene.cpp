#include "pch.h"
#include "LoadingScene.h"
#include "../Game/System/Timer.h"

LoadingScene::LoadingScene(std::shared_ptr<RenderManager> renderMgr) {
	mLoading.Init(renderMgr->GetCanvas(), renderMgr->GetTextureManager().GetTexture("Load"), 12, 4, 1.f);

	mLoading.GetRect().width = static_cast<float>(100);
	mLoading.GetRect().height = static_cast<float>(100);
	mLoading.GetRect().LTx = (Config::WINDOW_WIDTH<float> - mLoading.GetRect().width) * 0.5f;
	mLoading.GetRect().LTy = (Config::WINDOW_HEIGHT<float> - mLoading.GetRect().height) * 0.5f + 300.f;

	mBackground.Init(renderMgr->GetCanvas(), renderMgr->GetTextureManager().GetTexture("LoadingBG0"));

	mLoadingWord.Init(renderMgr->GetCanvas(), renderMgr->GetTextureManager().GetTexture("loading"));
	mLoadingWord.GetRect() = { Config::WINDOW_WIDTH<float> / 2.f - 512.f, 100.f, 1024.f, 170.f };
}

LoadingScene::~LoadingScene() {
}

void LoadingScene::Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {

}

void LoadingScene::ProcessNetwork() {
}

void LoadingScene::Update() {
	mBackground.Update();
	mLoadingWord.Update();
	mLoading.Update();
}

void LoadingScene::SendNetwork() {
}

void LoadingScene::Exit() {

}
