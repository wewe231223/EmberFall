#pragma once 
#include "../Renderer/Render/Canvas.h"
#include <functional>
class Button {
public:
	enum InvokeCondition : BYTE {
		LeftClick,
		RightClick,
	};
public:
	Button() = default;
	~Button() = default;

public:
	void Init(Canvas& canvas, UINT idle, UINT pressed);
	
	void Update(); 
	void SetCallBack(std::function<void()> callback, InvokeCondition condition);

	void SetRect(float LTx, float LTy, float width, float height); 

private:
	CanvasObject mButton{}; 

	InvokeCondition mCondition{ InvokeCondition::RightClick };
	std::function<void()> mCallback{ nullptr };


	bool mPressedRight{ false };
	bool mPressedLeft{ false };
};
