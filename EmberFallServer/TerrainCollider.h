#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TerrainCollider.h
// 
// 2025 - 02 - 26 김성준 : CollisionWorld삭제, Terrain과의 충돌처리만 남김.
//                         TerrainCollider 로 이름 변경
// 
//        04 - 20 김성준 : 멀티쓰레드 루프로 변경함에 따라 오브젝트를 저장하던 vector 삭제
//                         각 오브젝트가 자신의 업데이트 중에 터레인과의 충돌처리 함수를 호출
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
    void HandleTerrainCollision(std::shared_ptr<GameObject>& obj);

private:
    std::shared_ptr<Terrain> mTerrain{ };
};

