#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TerrainCollider.h
// 
// 2025 - 02 - 26 김성준 : CollisionWorld삭제, Terrain과의 충돌처리만 남김.
//                         TerrainCollider 로 이름 변경
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GameObject;
class Terrain;

class TerrainCollider {
public:
    TerrainCollider();
    ~TerrainCollider();

    TerrainCollider(const TerrainCollider&) = delete;
    TerrainCollider(TerrainCollider&&) noexcept = delete;
    TerrainCollider& operator=(const TerrainCollider&) = delete;
    TerrainCollider& operator=(TerrainCollider&&) noexcept = delete;

public:
    void SetTerrain(std::shared_ptr<Terrain> terrain);
    void AddObjectInTerrainGroup(std::shared_ptr<GameObject> object);

    void RemoveObjectFromTerrainGroup(std::shared_ptr<GameObject> obj);

    void HandleTerrainCollision();

private:
    std::shared_ptr<Terrain> mTerrain{ };
    std::vector<std::shared_ptr<GameObject>> mCollisionTerrainList{ };
};

