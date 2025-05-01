#pragma once 

#include "../Renderer/Render/Canvas.h"

class Profile {
public:
	Profile() = default;
	~Profile() = default;
public:
	void Init(std::shared_ptr<Canvas> canvas, UINT frame, UINT profile);
	void Update();
private:
	CanvasObject mBaseFrame{};
	CanvasObject mProfile{};
};