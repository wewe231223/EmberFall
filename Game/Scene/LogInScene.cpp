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
		500.f, 650.f, 500.f, 100.f
	);

	mPWEntry.Init(
		mRenderManager->GetCanvas(),
		mRenderManager->GetTextureManager().GetTexture("mid_dark_bar"),
		mRenderManager->GetTextureManager().GetTexture("Carrot"),
		500.f, 800.f, 500.f, 100.f
	);

	mBackgroundImage.Init(
		mRenderManager->GetCanvas(),
		mRenderManager->GetTextureManager().GetTexture("Title")
	);

	mID = TextBlockManager::GetInstance().CreateTextBlock(L"ID : ",
		D2D1_RECT_F{
			500.f - 100.f,			// left
			660.f,                 // top
			500.f - 20.f,          // right
			660.f + 100.f          // bottom
		}, StringColor::White, "NotoSansKR_Big");

	mPW = TextBlockManager::GetInstance().CreateTextBlock(L"PW : ",
		D2D1_RECT_F{
			500.f - 130.f,  // left
			810.f,                 // top
			500.f - 20.f,          // right
			810.f + 100.f          // bottom
		}, StringColor::White, "NotoSansKR_Big");

	mLoginButton.Init(
		mRenderManager->GetCanvas(),
		Button::InvokeCondition::LeftClick,
		mRenderManager->GetTextureManager().GetTexture("Login")
	);

	mLoginButton.SetRect(1050.f, 650.f, 250.f, 250.f);

}

void LogInScene::ProcessNetwork() {

}

void LogInScene::Update() {
	mBackgroundImage.Update();
	mIDEntry.Update();
	mPWEntry.Update();
	mLoginButton.Update();
}

void LogInScene::SendNetwork() {

}

void LogInScene::Exit() {
	mIDEntry.SetActiveState(false);
	mPWEntry.SetActiveState(false);
	mIDEntry.SetActiveState(false);
	mPWEntry.SetActiveState(false);
}
