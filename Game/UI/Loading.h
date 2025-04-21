#pragma once 
#include "../Renderer/Render/Canvas.h"

class Loading {
public:
	Loading() = default;
	~Loading() = default;

public:
	void Init(std::shared_ptr<Canvas> canvas, UINT img, UINT frameinRow, UINT frameinCol, float duration);

	void Update(); 
private:
	CanvasObject mLoading{}; 
};