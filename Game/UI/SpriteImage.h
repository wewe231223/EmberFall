#pragma once 
#include "../Renderer/Render/Canvas.h"

class SpriteImage {
public:
	SpriteImage() = default;
	~SpriteImage() = default;

public:
	void Init(Canvas& canvas, UINT img, UINT frameinRow, UINT frameinCol, float duration);
	CanvasRect& GetRect();

	void Update(); 
private:
	CanvasObject mObject{}; 
};