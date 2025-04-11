#include "pch.h"
#include "GridWorld.h"
#include "GameObject.h"
#include "ServerGameScene.h"

GridWorld::GridWorld(float cellWidth, float cellHeight, float mapWidth, float mapHeight, SimpleMath::Vector2 center) 
    : mGridWidth{ cellWidth }, mGridHeight{ cellHeight } {
    mWidth = static_cast<INT32>(std::ceil(mapWidth / cellWidth));
    mHeight = static_cast<INT32>(std::ceil(mapHeight / cellHeight));
    SimpleMath::Vector2 mapLT{ -mapWidth / 2.0f, -mapHeight / 2.0f };

    for (size_t y = 0; y < mWidth; ++y) {
        for (size_t x = 0; x < mHeight; ++x) {
            SimpleMath::Vector2 gridPos{ x * cellWidth, y * cellHeight };
            mGrid.emplace_back(mapLT + gridPos, cellWidth, cellHeight);
        }
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GridMap Generated!");
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Grid Width, Height: ({}, {})", cellWidth, cellHeight);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Map Size: ({}, {})", mWidth, mHeight);
}

GridWorld::~GridWorld() { }

void GridWorld::Update(const std::vector<std::shared_ptr<class GameObject>>& objects) {
    for (auto& grid : mGrid) {
        grid.Clear();
    }

    for (auto& player : objects) {
        if (not player->mSpec.active or not player->IsCollidingObject()) {
            continue;
        }

        decltype(auto) obb = std::static_pointer_cast<OrientedBoxCollider>(player->GetCollider())->GetBoundingBox();
        auto aabbMin = obb.Center - (SimpleMath::Vector3{ obb.Extents } * 1.415f); // root 2 (OBB를 감싸는 최소한의 AABB)
        auto aabbMax = obb.Center + (SimpleMath::Vector3{ obb.Extents } * 1.415f);

        INT32 minX = std::max(static_cast<INT32>(aabbMin.x / mGridWidth), 0);
        INT32 maxX = std::min(static_cast<INT32>(aabbMax.x / mGridWidth), mWidth);
        INT32 minZ = std::max(static_cast<INT32>(aabbMin.z / mGridHeight), 0);
        INT32 maxZ = std::min(static_cast<INT32>(aabbMax.z / mGridHeight), mHeight);

        for (INT32 z = minZ; z <= maxZ; ++z) {
            for (INT32 x = minX; x <= maxX; ++x) {
                mGrid[z * mHeight + x].AddObject(player);
            }
        }
    }

    for (auto& grid : mGrid) {
        grid.HandleCollision();
    }
}
