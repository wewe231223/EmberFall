#pragma once

class Grid {
public:
    Grid(SimpleMath::Vector2 position, float cellWidth, float cellHeight);
    ~Grid();

public:
    void HandleCollision();

public:
    void Clear();
    bool Intersects(const std::shared_ptr<class Collider>& collider);
    void AddObject(const std::shared_ptr<class GameObject>& object);

private:
    SimpleMath::Vector2 mCellPosition{ };
    SimpleMath::Vector2 mCellSize{ };
    std::vector<std::shared_ptr<class GameObject>> mObjects{ };
};
