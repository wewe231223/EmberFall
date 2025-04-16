#include "pch.h"
#include "Inventory.h"

void Inventory::Init(std::shared_ptr<Canvas> canvas, UINT base, UINT frame, UINT health, UINT cross, UINT holyWater) {
	for(auto i = 0; i < 3; ++i) {
		auto& pair = mItem[i];
		pair.first = canvas->CreateCanvasObject();
		pair.second = canvas->CreateCanvasObject();

		pair.first.ChangeImage(frame);
		pair.first.GetRect() = { 760.f + i * 200.f, 880.f, 200.f, 200.f };

		pair.second.ChangeImage(base);
		pair.second.GetRect() = { 760.f + i * 200.f, 880.f, 200.f, 200.f };
	}
}

void Inventory::Update() {
	for (auto& pair : mItem) {
		pair.second.Update();
		pair.first.Update();
	}
}
