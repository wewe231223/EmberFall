#include "pch.h"
#include "GridWorld.h"
#include "GameObject.h"

GridWorld::GridWorld(float cellWidth, float cellHeight, float mapWidth, float mapHeight, SimpleMath::Vector2 center) 
    : mGridWidth{ cellWidth }, mGridHeight{ cellHeight } {
    mWidth = std::ceil(mapWidth / cellWidth);
    mHeight = std::ceil(mapHeight / cellHeight);
    SimpleMath::Vector2 mapLT{ -mapWidth / 2.0f, -mapHeight / 2.0f };

    for (size_t y = 0; y < mWidth; ++y) {
        for (size_t x = 0; x < mHeight; ++x) {
            SimpleMath::Vector2 gridPos{ x * cellWidth, y * cellHeight };
            mGrid.emplace_back(mapLT + gridPos, cellWidth, cellHeight);
        }
    }
}

GridWorld::~GridWorld() { }

void GridWorld::Update(const std::vector<std::shared_ptr<class GameObject>>& objects) {
    for (auto& grid : mGrid) {
        grid.Clear();
    }

    for (auto& object : objects) {
        decltype(auto) obb = std::static_pointer_cast<OrientedBoxCollider>(object->GetCollider())->GetBoundingBox();
        auto aabbMin = obb.Center - (SimpleMath::Vector3{ obb.Extents } * 1.415);
        auto aabbMax = obb.Center + (SimpleMath::Vector3{ obb.Extents } * 1.415);

        INT32 x1 = std::max(static_cast<INT32>(aabbMin.x / mGridWidth), 0);
        INT32 x2 = std::min(static_cast<INT32>(aabbMax.x / mGridWidth), mWidth);
        INT32 y1 = std::max(static_cast<INT32>(aabbMin.y / mHeight), 0);
        INT32 y2 = std::min(static_cast<INT32>(aabbMax.y / mHeight), mHeight);

        for (INT32 y = y1; y <= y2; ++y) {
            for (INT32 x = x1; x <= x2; ++x) {
                mGrid[y * mGridHeight + x].AddObject(object);
            }
        }
    }

    for (auto& grid : mGrid) {
        grid.HandleCollision();
    }
}
