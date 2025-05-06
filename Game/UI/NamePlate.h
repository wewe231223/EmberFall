#pragma once 
#include "../Renderer/Render/Canvas.h"
#include "../Renderer/Core/StringRenderer.h"

class NamePlate {
public:
	NamePlate() = default; 
	~NamePlate() = default; 

public:
	void Init(Canvas& canvas, UINT frame, float LTx, float LTy, float width, float height);

	void SetName(const char* str); 
	void SetActiveState(bool state);

	void Update(); 

	std::wstring& GetName(); 
private:
	CanvasObject mFrame{}; 
	TextBlock* mName{}; 
};