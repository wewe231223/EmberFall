#include "pch.h"
#include "CollisionWorld.h"
#include "GameObject.h"
#include "Collider.h"
#include "Terrain.h"
#include "Physics.h"

CollisionWorld::CollisionWorld() { }

CollisionWorld::~CollisionWorld() { }

void CollisionWorld::AddTerrain(std::shared_ptr<Terrain> terrain) {
    mTerrain = terrain;
}

void CollisionWorld::AddObjectInTerrainGroup(std::shared_ptr<GameObject> object) {
    mCollisionTerrainList.push_back(object);
}

bool CollisionWorld::Contains(const std::string& groupTag) const {
    return mCollisionWorld.contains(groupTag);
}

void CollisionWorld::AddCollisionPair(const std::string& groupTag, std::shared_ptr<GameObject> obj1, std::shared_ptr<GameObject> obj2) {
    if (nullptr == obj1 and nullptr == obj2) {
        return;
    }

    if (false == Contains(groupTag)) {
        mCollisionWorld[groupTag] = CollisionPair{ };
    }

    if (nullptr != obj1) {
        CollisionList& firstList = mCollisionWorld[groupTag].first;
        auto obj1Search = std::find(firstList.begin(), firstList.end(), obj1);
        if (obj1Search == firstList.end()) {
            firstList.push_back(obj1);
        }
    }

    if (nullptr != obj2) {
        CollisionList& secondList = mCollisionWorld[groupTag].second;
        auto obj2Search = std::find(secondList.begin(), secondList.end(), obj1);
        if (obj2Search == secondList.end()) {
            secondList.push_back(obj2);
        }
    }
}

void CollisionWorld::AddCollisionObject(const std::string& groupTag, std::shared_ptr<GameObject> obj) {
    if (nullptr == obj) {
        return;
    }

    if (false == Contains(groupTag)) {
        mCollisionWorld[groupTag] = CollisionPair{ };
    }

    mCollisionWorld[groupTag].first.push_back(obj);
}

void CollisionWorld::RemoveObjectFromGroup(const std::string& groupTag, std::shared_ptr<GameObject> obj) {
    if (nullptr == obj) {
        return;
    }

    if (false == Contains(groupTag)) {
        return;
    }

    CollisionList& firstList = mCollisionWorld[groupTag].first;
    auto objSearch = std::find(firstList.begin(), firstList.end(), obj);
    if (objSearch != firstList.end()) {
        std::swap(*objSearch, firstList.back());
        firstList.pop_back();
        return;
    }

    CollisionList& secondList = mCollisionWorld[groupTag].second;
    objSearch = std::find(secondList.begin(), secondList.end(), obj);
    if (objSearch != secondList.end()) {
        std::swap(*objSearch, secondList.back());
        secondList.pop_back();
        return;
    }
}

void CollisionWorld::RemoveObjectFromTerrainGroup(std::shared_ptr<GameObject> obj) {
    auto objSearch = std::find(mCollisionTerrainList.begin(), mCollisionTerrainList.end(), obj);
    if (objSearch != mCollisionTerrainList.end()) {
        std::erase(mCollisionTerrainList, *objSearch);
        return;
    }
}

void CollisionWorld::HandleCollision() {
    for (auto& [group, listPair] : mCollisionWorld) {
        if (listPair.second.empty()) {  // 두번째 리스트를 사용하지 않으면 같은 종류의 오브젝트끼리만 충돌체크 (첫번째 리스트만 사용)
            HandleCollisionList(group, listPair.first);
        }
        else {
            HandleCollisionListPair(group, listPair);
        }
    }
}

void CollisionWorld::HandleTerrainCollision() {
    float terrainHeight{ };
    bool onGround{ false };
    for (auto& object : mCollisionTerrainList) {
        onGround = false;
        if (mTerrain->Contains(object->GetCollider(), terrainHeight)) {
            onGround = true;
            object->GetPhysics()->mFactor.friction = 1.0f;
            object->OnCollisionTerrain(terrainHeight);
        }

        object->GetPhysics()->SetOnGround(onGround);
    }
}

void CollisionWorld::HandleCollision(const std::string& groupTag, std::shared_ptr<GameObject>& obj1, std::shared_ptr<GameObject>& obj2) {
    auto collider1 = obj1->GetCollider();
    auto collider2 = obj2->GetCollider();

    auto collisionResult = collider1->CheckCollision(collider2);
    collider1->UpdateState(collisionResult, obj2->GetId());
    collider2->UpdateState(collisionResult, obj1->GetId());

    obj1->OnCollision(groupTag, obj2);
    obj2->OnCollision(groupTag, obj1);
}

void CollisionWorld::HandleCollisionListPair(const std::string& groupTag, CollisionPair& listPair) {
    for (auto& obj1 : listPair.first) {
        for (auto& obj2 : listPair.second) {
            HandleCollision(groupTag, obj1, obj2);
        }
    }
}

void CollisionWorld::HandleCollisionList(const std::string& groupTag, CollisionList& list) {
    auto listIter = list.begin();
    auto listEnd = list.end();
    for (; listIter != listEnd; ++listIter) { // iterator
        for (auto next = listIter + 1; next != listEnd; ++next) { // iterator + 1 ~ end 
            // Collision Check { nth object, (n + 1)th object }
            auto obj1 = *listIter;
            auto obj2 = *next;

            HandleCollision(groupTag, obj1, obj2);
        }
    }
}
