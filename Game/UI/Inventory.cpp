#include "pch.h"
#include "Inventory.h"

void Inventory::Init(Canvas& canvas, UINT base, UINT frame, UINT health, UINT cross, UINT holyWater) {
	float screenWidth = Config::WINDOW_WIDTH<float>;
	float screenHeight = Config::WINDOW_HEIGHT<float>;

	constexpr int ITEM_COUNT = 5;
	constexpr float rectWidth = 100.f;
	constexpr float rectHeight = 100.f;

	constexpr float framePaddingX = 12.f; // 가로 방향 패딩
	constexpr float framePaddingY = 2.f;  // 세로 방향 패딩

	float totalWidth = ITEM_COUNT * rectWidth;
	float frameWidth = totalWidth + framePaddingX * 2.f;
	float frameHeight = rectHeight + framePaddingY * 2.f;

	float frameX = (screenWidth - frameWidth) / 2.f;
	float frameY = screenHeight - frameHeight;

	mBaseFrame = canvas.CreateCanvasObject();
	mBaseFrame.ChangeImage(base);
	mBaseFrame.GetRect() = { frameX, frameY, frameWidth, frameHeight };

	float startX = frameX + framePaddingX;
	float posY = frameY + framePaddingY;

	mItem.resize(ITEM_COUNT);

	for (int i = 0; i < ITEM_COUNT; ++i) {
		auto& pair = mItem[i];
		pair.first = canvas.CreateCanvasObject();
		pair.second = canvas.CreateCanvasObject();

		pair.first.ChangeImage(frame);
		pair.second.SetActive(false);

		float x = startX + i * rectWidth;
		pair.first.GetRect() = { x, posY, rectWidth, rectHeight };
		pair.second.GetRect() = { x, posY, rectWidth, rectHeight };
	}

	mItemImage[static_cast<int>(ItemType::Health)] = health;
	mItemImage[static_cast<int>(ItemType::Cross)] = cross;
	mItemImage[static_cast<int>(ItemType::HolyWater)] = holyWater;
}

void Inventory::SetItem(ItemType type, UINT slot, bool active) {
	if (not active) {
		mItem[slot].second.SetActive(false);
		return;
	}
	mItem[slot].second.ChangeImage(mItemImage[static_cast<int>(type)]);
	mItem[slot].second.SetActive(true);
}

void Inventory::DisactiveItem(UINT slot) {
	mItem[slot].second.SetActive(false);
}

void Inventory::Update() {
	mBaseFrame.Update();

	for (auto& pair : mItem) {
		pair.second.Update();
		pair.first.Update();
	}
}
