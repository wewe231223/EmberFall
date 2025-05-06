#include "pch.h"
#include "Button.h"
#include "../Game/System/Input.h"

void Button::Init(Canvas& canvas, InvokeCondition condition, UINT image) {
	mButton = canvas.CreateCanvasObject();
	mButton.ChangeImage(image);
	mButton.GetRect() = { 0.f, 0.f, 100.f, 100.f };

	mButton.SetGreyScale(IDLE_GREYSCALE);
	mButton.SetActive(true);

	mCallback = []() {};
}

void Button::Update() {
	if (not mButton.GetActive()) {
		return;
	}

	auto& rect = mButton.GetRect(); 

	if (Button::Inside(static_cast<int>(rect.LTx), static_cast<int>(rect.LTx + rect.width), Input.GetMouseState().x) and
		Button::Inside(static_cast<int>(rect.LTy), static_cast<int>(rect.LTy + rect.height), Input.GetMouseState().y)) {
		if (mCondition == LeftClick) {
			if (Input.GetMouseTracker().leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED and mPressedLeft and mCondition == LeftClick) {
				mPressedLeft = false;
				std::invoke(mCallback);
				mButton.SetGreyScale(IDLE_GREYSCALE);
			}
			else if (Input.GetMouseTracker().leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
				mPressedLeft = true;
				mButton.SetGreyScale(CLICK_GREYSCALE);
			}
			else if (not mPressedLeft) {
				mButton.SetGreyScale(HOVER_GREYSCALE);
			}
		}
		else if (mCondition== RightClick) {
			if (Input.GetMouseTracker().rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED and mPressedRight and mCondition == RightClick) {
				mPressedRight = false;
				std::invoke(mCallback);
				mButton.SetGreyScale(IDLE_GREYSCALE);
			}
			else if (Input.GetMouseTracker().rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
				mPressedRight = true;
				mButton.SetGreyScale(CLICK_GREYSCALE);
			}
			else if (not mPressedRight) {
				mButton.SetGreyScale(HOVER_GREYSCALE);
			}
		}

	}
	else {
		if (mPressedLeft or mPressedRight) {
			mPressedLeft = false;
			mPressedRight = false;
		}
		mButton.SetGreyScale(IDLE_GREYSCALE);
	}

	mButton.Update(); 
}

void Button::SetCallBack(std::function<void()> callback, InvokeCondition condition) {
	mCondition = condition; 
	mCallback = callback;
}

void Button::SetRect(float LTx, float LTy, float width, float height) {
	auto& rect = mButton.GetRect();
	rect = { LTx, LTy, width, height };
}

void Button::ChangeImage(UINT image) {
	mButton.ChangeImage(image);
}

void Button::SetActiveState(bool state) {
	mButton.SetActive(state);
}

bool Button::Inside(int min, int max, int value) {
	return ((value > min) and (value < max));
}
