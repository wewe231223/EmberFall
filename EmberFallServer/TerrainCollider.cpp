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

void TerrainCollider::SetTerrain(const std::filesystem::path& filePath) {
    mTerrain = std::make_shared<Terrain>(filePath);
}

void TerrainCollider::HandleTerrainCollision(const std::shared_ptr<GameObject>& obj) {
    float terrainHeight{ };
    bool onGround{ false };

    obj->GetPhysics()->mFactor.friction = 0.0f;
    auto boundingObj = obj->GetBoundingObject();
    if (nullptr == boundingObj) {
        return;
    }

    if (nullptr == mTerrain) {
        obj->OnCollisionTerrain(0.0f);
        return;
    }

    if (mTerrain->Contains(boundingObj, terrainHeight)) {
        onGround = true;
        obj->GetPhysics()->mFactor.friction = 1.0f;
        obj->OnCollisionTerrain(terrainHeight);
    }

    obj->GetPhysics()->SetOnGround(onGround);
}