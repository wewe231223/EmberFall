#include "pch.h"
#include "HealthBar.h"

const float healthPaddingX = 10.f; // 가로 방향 패딩
const float healthPaddingY = 10.f; // 세로 방향 패딩

void HealthBar::Init(std::shared_ptr<Canvas> canvas, UINT frame, UINT health) {
	mBaseFrame = canvas->CreateCanvasObject();
	mBaseFrame.ChangeImage(frame);
	mBaseFrame.GetRect() = { 191.f, 50.f, 630.f, 50.f };

	mHealthBar = canvas->CreateCanvasObject();
	mHealthBar.ChangeImage(health);
	mHealthBar.GetRect() = { 
		mBaseFrame.GetRect().LTx + healthPaddingX, 
		mBaseFrame.GetRect().LTy + healthPaddingY,
		mBaseFrame.GetRect().width - healthPaddingX * 2.f, 
		mBaseFrame.GetRect().height - healthPaddingY * 2.f 
	};
}

void HealthBar::Update() {
	mBaseFrame.Update();
	mHealthBar.Update(); 
}

void HealthBar::SetHealth(float health) {
	mHealth = health;

	mHealthBar.GetRect() = {
		mBaseFrame.GetRect().LTx + healthPaddingX,
		mBaseFrame.GetRect().LTy + healthPaddingY,
		(mBaseFrame.GetRect().width - healthPaddingX * 2.f) * (mHealth / 100.f),
		mBaseFrame.GetRect().height - healthPaddingY * 2.f
	};
}
