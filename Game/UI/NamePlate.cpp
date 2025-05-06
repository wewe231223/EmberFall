#include "pch.h"
#include "NamePlate.h"

void NamePlate::Init(Canvas& canvas, UINT frame, float LTx, float LTy, float width, float height) {
	mFrame = canvas.CreateCanvasObject(); 
	mFrame.GetRect() = { LTx, LTy, width, height };
	mFrame.ChangeImage(frame);

	mName = TextBlockManager::GetInstance().CreateTextBlock(L"", D2D1_RECT_F{ LTx, LTy, LTx + width, LTy + height }, StringColor::White, "NotoSansKR");
	mName->GetText() = L"";
	mName->GetRect() = { LTx, LTy, LTx + width, LTy + height };
}

void NamePlate::SetName(const char* str) {
	mName->GetText() = ConvertUtf8ToWstring(str);
}

void NamePlate::SetActiveState(bool state) {
	mFrame.SetActive(state);
	mName->SetActiveState(state);
}

void NamePlate::Update() {
	mFrame.Update(); 
}

std::wstring& NamePlate::GetName() {
	return mName->GetText(); 
}
