#include "pch.h"
#include "LogInScene.h"

LogInScene::LogInScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation) {
	mRenderManager = renderMgr;
}

LogInScene::~LogInScene() {

}

void LogInScene::Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mIDEntry.Init(
		mRenderManager->GetCanvas(),
		mRenderManager->GetTextureManager().GetTexture("mid_dark_bar"),
		mRenderManager->GetTextureManager().GetTexture("Carrot"),
		500.f, 500.f, 500.f, 100.f
	);

	mPWEntry.Init(
		mRenderManager->GetCanvas(),
		mRenderManager->GetTextureManager().GetTexture("mid_dark_bar"),
		mRenderManager->GetTextureManager().GetTexture("Carrot"),
		500.f, 650.f, 500.f, 100.f
	);
}

void LogInScene::ProcessNetwork() {

}

void LogInScene::Update() {
	mIDEntry.Update();
	mPWEntry.Update();
}

void LogInScene::SendNetwork() {

}

void LogInScene::Exit() {

}
