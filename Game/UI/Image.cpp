#include "pch.h"
#include "Image.h"

void Image::Init(Canvas& canvas, UINT img) {
	mObject = canvas.CreateCanvasObject();
	mObject.ChangeImage(img);
	mObject.SetActive(true);

	mObject.GetRect() = { 0.f, 0.f, Config::WINDOW_WIDTH<float>, Config::WINDOW_HEIGHT<float> };
}

CanvasRect& Image::GetRect() {
	return mObject.GetRect();
}

void Image::Update() {
	mObject.Update();
}
