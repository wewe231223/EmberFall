#pragma once 
#include "../Renderer/Render/Canvas.h"

class Image {
public:
	Image() = default;
	~Image() = default;

public:
	void Init(std::shared_ptr<Canvas> canvas, UINT img);
	CanvasRect& GetRect();

	void Update();
private:
	CanvasObject mObject{};
};