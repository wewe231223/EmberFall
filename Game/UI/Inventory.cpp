#include "pch.h"
#include "Inventory.h"

void Inventory::Init(std::shared_ptr<Canvas> canvas, UINT base, UINT frame, UINT health, UINT cross, UINT holyWater) {
	float screenCenterX = Config::WINDOW_WIDTH<float> / 2.f;
	float screenBottomY = Config::WINDOW_HEIGHT<float>;
	
	constexpr float rectWidth = 100.f;
	constexpr float rectHeight = 100.f;
	
	float totalWidth = rectWidth * 3.f;
	float startX = screenCenterX - totalWidth / 2.f;
	float posY = screenBottomY - rectHeight; 

	for (int i = 0; i < 3; ++i) {
		auto& pair = mItem[i];
		pair.first = canvas->CreateCanvasObject();
		pair.second = canvas->CreateCanvasObject();

		pair.first.ChangeImage(frame);
		pair.second.ChangeImage(base);

		float x = startX + i * rectWidth;
		pair.first.GetRect() = { x, posY, rectWidth, rectHeight };
		pair.second.GetRect() = { x, posY, rectWidth, rectHeight };
	}

}

void Inventory::Update() {
	for (auto& pair : mItem) {
		pair.second.Update();
		pair.first.Update();
	}
}
