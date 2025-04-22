#include "pch.h"
#include "SpriteImage.h"
#include "../Game/System/Timer.h"

void SpriteImage::Init(std::shared_ptr<Canvas> canvas, UINT img, UINT frameinRow, UINT frameinCol, float duration) {
	mObject = canvas->CreateCanvasObject();
	mObject.ChangeImage(img, frameinRow, frameinCol, duration);
	mObject.SetActive(true);


}

CanvasRect& SpriteImage::GetRect() {
	return mObject.GetRect(); 
}

void SpriteImage::Update() {
	mObject.Update(); 
}
