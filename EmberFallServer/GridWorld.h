#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GridWorld.h
// 
// 2025 - 02 - 05 김성준 : 간단한 공간분할
//                          가로 세로 일정 길이 만큼 공간을 분할.
//                          그 안에 오브젝트가 존재하는 지는 Grid 클래스의 Intersect 함수로 판별
// 
//                          성능은 약 3배정도 상승하는 것으로 보임.              
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Grid.h"

class GridWorld {
public:
    GridWorld(float cellWidth, float cellHeight, float mapWidth, float mapHeight, SimpleMath::Vector2 center = SimpleMath::Vector2::Zero);
    ~GridWorld();

public:
    void Update(const std::shared_ptr<class IServerGameScene>& gameScene);
    void Update(const std::vector<std::shared_ptr<class GameObject>>& objects);

private:
    std::vector<Grid> mGrid{ };
    float mGridWidth{ };
    float mGridHeight{ };

    INT32 mWidth{ };
    INT32 mHeight{ };
};

