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

	mCanvas = &canvas;
}

void Button::Update() {
	if (not mButton.GetActive()) {
		return;
	}

	RECT clientRect{ mCanvas->GetClientRect() };
	const float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	const float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	constexpr float renderWidth{ Config::WINDOW_WIDTH<float> };
	constexpr float renderHeight{ Config::WINDOW_HEIGHT<float> };

	const float mouseX = Input.GetMouseState().x * renderWidth / clientWidth;
	const float mouseY = Input.GetMouseState().y * renderHeight / clientHeight;

	auto& rect = mButton.GetRect();

	if (Button::Inside(rect.LTx, rect.LTx + rect.width, mouseX) && Button::Inside(rect.LTy, rect.LTy + rect.height, mouseY)) {

		if (mCondition == LeftClick) {
			if (Input.GetMouseTracker().leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED && mPressedLeft) {
				mPressedLeft = false;
				std::invoke(mCallback);
				mButton.SetGreyScale(IDLE_GREYSCALE);
			}
			else if (Input.GetMouseTracker().leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
				mPressedLeft = true;
				mButton.SetGreyScale(CLICK_GREYSCALE);
			}
			else if (!mPressedLeft) {
				mButton.SetGreyScale(HOVER_GREYSCALE);
			}
		}
		else if (mCondition == RightClick) {
			if (Input.GetMouseTracker().rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED && mPressedRight) {
				mPressedRight = false;
				std::invoke(mCallback);
				mButton.SetGreyScale(IDLE_GREYSCALE);
			}
			else if (Input.GetMouseTracker().rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
				mPressedRight = true;
				mButton.SetGreyScale(CLICK_GREYSCALE);
			}
			else if (!mPressedRight) {
				mButton.SetGreyScale(HOVER_GREYSCALE);
			}
		}

	}
	else {
		mPressedLeft = mPressedRight = false;
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

bool Button::Inside(float min, float max, float value) {
	return ((value > min) and (value < max));
}
