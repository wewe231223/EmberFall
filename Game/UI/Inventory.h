#pragma once 

#include "../Renderer/Render/Canvas.h"

enum class ItemType : BYTE {
	Health = 0,
	Cross = 1,
	HolyWater = 2,
};

class Inventory {
public:
	Inventory() = default; 
	~Inventory() = default;
public:
	void Init(std::shared_ptr<Canvas> canvas, UINT base, UINT frame, UINT health, UINT cross, UINT holyWater);
	void Update(); 
private:
	std::array<std::pair<CanvasObject, CanvasObject>, 3> mItem{};
	std::array<UINT, 3> mItemImage{}; 
};