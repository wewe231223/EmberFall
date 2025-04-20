#pragma once 
#include "../Renderer/Render/Canvas.h"

class Loading {
public:
	Loading();
	~Loading();

public:
	void Init(std::shared_ptr<Canvas> canvas, UINT img);

	void Update(); 
private:
	CanvasObject mLoading{}; 
};