#include "pch.h"
#include "TerrainCollider.h"
#include "GameObject.h"
#include "Collider.h"
#include "Terrain.h"
#include "Physics.h"

TerrainCollider::TerrainCollider() { }

TerrainCollider::~TerrainCollider() { }

void TerrainCollider::SetTerrain(std::shared_ptr<Terrain> terrain) {
    mTerrain = terrain;
}

void TerrainCollider::AddObjectInTerrainGroup(std::shared_ptr<GameObject> object) {
    mCollisionTerrainList.push_back(object);
}

void TerrainCollider::RemoveObjectFromTerrainGroup(std::shared_ptr<GameObject> obj) {
    auto objSearch = std::find(mCollisionTerrainList.begin(), mCollisionTerrainList.end(), obj);
    if (objSearch != mCollisionTerrainList.end()) {
        std::erase(mCollisionTerrainList, *objSearch);
        return;
    }
}

void TerrainCollider::HandleTerrainCollision() {
    float terrainHeight{ };
    bool onGround{ false };
    for (auto& object : mCollisionTerrainList) {
        onGround = false;
        object->GetPhysics()->mFactor.friction = 0.0f;
        if (mTerrain->Contains(object->GetCollider(), terrainHeight)) {
            onGround = true;
            object->GetPhysics()->mFactor.friction = 1.0f;
            object->OnCollisionTerrain(terrainHeight);
        }

        object->GetPhysics()->SetOnGround(onGround);
    }
}