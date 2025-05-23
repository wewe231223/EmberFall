#pragma once 
#include "../Renderer/Render/Canvas.h"

class Image {
public:
	Image() = default;
	~Image() = default;

public:
	void Init(Canvas& canvas, UINT img);
	CanvasRect& GetRect();

	void Update();

	void SetActiveState(bool state);
private:
	CanvasObject mObject{};
};