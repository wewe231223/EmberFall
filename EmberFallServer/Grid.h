#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Grid.h
// 
// 2025 - 02 - 05 김성준 : 간단한 공간분할
//                          전체 맵을 분할 한 것들중에 하나.
//                          실제 오브젝트들이 등록된 클래스이고 여기서 실제 충돌 처리를 진행함.
//  
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Grid {
public:
    Grid(SimpleMath::Vector2 position, float cellWidth, float cellHeight);
    ~Grid();

public:
    void HandleCollision();

public:
    void Clear();
    void AddObject(const std::shared_ptr<class GameObject>& object);

private:
    SimpleMath::Vector2 mCellPosition{ };
    SimpleMath::Vector2 mCellSize{ };
    std::vector<std::shared_ptr<class GameObject>> mObjects{ };
};
