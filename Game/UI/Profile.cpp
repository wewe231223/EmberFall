#include "pch.h"
#include "Profile.h"

void Profile::Init(std::shared_ptr<Canvas> canvas, UINT frame, UINT profile) {
	mBaseFrame = canvas->CreateCanvasObject();
	mBaseFrame.ChangeImage(frame);
	mBaseFrame.GetRect() = { 30.f, 30.f, 200.f, 200.f };

	mProfile = canvas->CreateCanvasObject();
	mProfile.ChangeImage(profile);
	mProfile.GetRect() = { 30.f, 30.f, 200.f, 200.f };
}

void Profile::Update() {
	mProfile.Update();
	mBaseFrame.Update();
}
