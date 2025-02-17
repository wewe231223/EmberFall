#pragma once

#include "Grid.h"

class GridWorld {
public:
    GridWorld(float cellWidth, float cellHeight, float mapWidth, float mapHeight, SimpleMath::Vector2 center = SimpleMath::Vector2::Zero);
    ~GridWorld();

public:
    void Update(const std::vector<std::shared_ptr<class GameObject>>& objects);

private:
    std::vector<Grid> mGrid{ };
    float mGridWidth{ };
    float mGridHeight{ };

    INT32 mWidth{ };
    INT32 mHeight{ };
};

