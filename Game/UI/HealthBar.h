#pragma once 
#include "../Renderer/Render/Canvas.h"


class HealthBar {
public:
	HealthBar() = default;
	~HealthBar() = default;

public:
	void Init(std::shared_ptr<Canvas> canvas, UINT frame, UINT health);

	void Update(); 

	void SetHealth(float health);
private:
	CanvasObject mBaseFrame{};
	CanvasObject mHealthBar{};

	float mHealth{ 100.f };
};