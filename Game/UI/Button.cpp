#include "pch.h"
#include "Button.h"

void Button::Init(Canvas& canvas, UINT idle, UINT pressed) {
	mButton = canvas.CreateCanvasObject();
	mButton.ChangeImage(idle);
	mButton.GetRect() = { 0.f, 0.f, 100.f, 100.f };

	

}

void Button::Update() {

}

void Button::SetCallBack(std::function<void()> callback, InvokeCondition condition) {

}

void Button::SetRect(float LTx, float LTy, float width, float height) {

}
