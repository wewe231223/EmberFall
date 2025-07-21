#pragma once 
#include "../Renderer/Render/Canvas.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"

class Entry {
public:
	Entry() = default;
	~Entry() = default;

public:
	void Init(Canvas& cancvas, UINT frame, UINT carrot, float LTx, float LTy, float width, float height, size_t max = 10);

	void SetActiveState(bool state);
	
	void Update(); 

	std::string GetText(); 
	CanvasRect& GetRect();
private:
	bool Inside(float min, float max, float value) const; 

	char GetPrintableChar(DirectX::Keyboard::Keys key, bool shift) const;

	std::string GetPressedText(); 

private:
	// 화면 보정을 위해서만 사용할 것 
	Canvas* mCanvas{ nullptr };

	CanvasObject mFrame{}; 
	CanvasObject mCarrot{};
	UINT mCarrotCount{ 100 };

	TextBlock* mText{}; 

	size_t maxLength{ 0 };

	bool mInputActive{ false };
};