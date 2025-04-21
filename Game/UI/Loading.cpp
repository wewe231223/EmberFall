#include "pch.h"
#include "Loading.h"

void Loading::Init(std::shared_ptr<Canvas> canvas, UINT img, UINT frameinRow, UINT frameinCol, float duration) {
	mLoading = canvas->CreateCanvasObject();
	mLoading.ChangeImage(img, frameinRow, frameinCol, duration);
	mLoading.SetActive(true);

	// Set the position and size of the loading screen
	mLoading.GetRect().LTx = 0.f;
	mLoading.GetRect().LTy = 0.f;
	mLoading.GetRect().width = static_cast<float>(500);
	mLoading.GetRect().height = static_cast<float>(500);
}

void Loading::Update() {
	mLoading.Update(); 
}
