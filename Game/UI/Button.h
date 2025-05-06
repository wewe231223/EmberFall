#pragma once 
#include "../Renderer/Render/Canvas.h"
#include <functional>
class Button {
	static constexpr float IDLE_GREYSCALE = 0.8f; 
	static constexpr float HOVER_GREYSCALE = 1.0f;
	static constexpr float CLICK_GREYSCALE = 0.3f;
public:
	enum InvokeCondition : BYTE {
		LeftClick,
		RightClick,
	};
public:
	Button() = default;
	~Button() = default;

public:
	void Init(Canvas& canvas, InvokeCondition condition, UINT image);
	
	void Update(); 
	void SetCallBack(std::function<void()> callback, InvokeCondition condition);

	void SetRect(float LTx, float LTy, float width, float height); 

	void ChangeImage(UINT image); 
	
	void SetActiveState(bool state); 
private:
	bool Inside(float min, float max, float value); 
private:
	Canvas* mCanvas{ nullptr }; // 화면 보정을 위해서만 사용 할 것. 
	CanvasObject mButton{}; 

	InvokeCondition mCondition{ InvokeCondition::RightClick };
	std::function<void()> mCallback{ nullptr };

	bool mPressedRight{ false };
	bool mPressedLeft{ false };
};
