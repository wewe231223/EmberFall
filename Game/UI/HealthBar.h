#pragma once 
#include "../Renderer/Render/Canvas.h"


class HealthBar {
public:
	HealthBar() = default;
	~HealthBar() = default;

public:
	void Init(Canvas& canvas, UINT frame, UINT health);

	void Update(); 

	void SetHealth(float health);
	void SetMaxHealth(float health); 
private:
	CanvasObject mBaseFrame{};
	CanvasObject mHealthBar{};

	float mHealth{ 100.f };
	float mMaxHealth{ 100.f }; 
};