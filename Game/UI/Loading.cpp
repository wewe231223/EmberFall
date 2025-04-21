#include "pch.h"
#include "Loading.h"

void Loading::Init(std::shared_ptr<Canvas> canvas, UINT img, UINT frameinRow, UINT frameinCol, float duration) {
	mLoading = canvas->CreateCanvasObject();
	mLoading.ChangeImage(img, frameinRow, frameinCol, duration);
	mLoading.SetActive(true);

	// Set the position and size of the loading screen
	mLoading.GetRect().width = static_cast<float>(400);
	mLoading.GetRect().height = static_cast<float>(400);
	mLoading.GetRect().LTx = static_cast<float>(Config::WINDOW_WIDTH<float> / 2.f - mLoading.GetRect().width / 2.f);
	mLoading.GetRect().LTy = static_cast<float>(Config::WINDOW_HEIGHT<float> - mLoading.GetRect().height - 50.f);
}

void Loading::Update() {
	mLoading.Update(); 
}
