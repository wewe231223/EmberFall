#pragma once 

#include "../Renderer/Render/Canvas.h"

enum class ItemType : BYTE {
	Health = 0,
	Cross = 1,
	HolyWater = 2,
};

class Inventory {
	static constexpr int ITEM_COUNT = 10; 
public:
	Inventory() = default; 
	~Inventory() = default;
public:
	void Init(Canvas& canvas, UINT base, UINT frame, UINT health, UINT cross, UINT holyWater);

	void SetItem(ItemType type, UINT slot, bool active); 

	void Update(); 
private:
	std::vector<std::pair<CanvasObject, CanvasObject>> mItem{};

	CanvasObject mBaseFrame{}; 

	std::array<UINT, 3> mItemImage{}; 
};