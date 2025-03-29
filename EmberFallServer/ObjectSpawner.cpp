#include "pch.h"
#include "ObjectSpawner.h"
#include "ObjectBuilder.h"

ObjectSpawner::ObjectSpawner() {}

ObjectSpawner::~ObjectSpawner() {

}

void ObjectSpawner::SetCurrentScene(std::shared_ptr<class IServerGameScene> gameScene) {
    mCurrentScene = gameScene;
}

std::shared_ptr<GameObject>& ObjectSpawner::SpawnObject(ObjectTag objectType, bool terrainCollision) {
    decltype(auto) object = mCurrentScene->GetInvalidObject();
    ObjectBuilder::BuildObjectComponent(object, objectType);
    
    object->SetActive(true);

    if (true == terrainCollision) {
        mCurrentScene->GetTerrainCollider().AddObjectInTerrainGroup(object);
    }

    return object;
}

std::shared_ptr<GameObject>& ObjectSpawner::SpawnTrigger(std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
    const SimpleMath::Vector3& pos, const SimpleMath::Vector3& size, const SimpleMath::Vector3& dir) {

    decltype(auto) object = mCurrentScene->GetInvalidObject();
    ObjectBuilder::BuildTrigger(object, event, lifeTime, eventDelay, eventCount, pos, size, dir);

    object->SetActive(true);
    return object;
}

std::shared_ptr<GameObject>& ObjectSpawner::SpawnTrigger(std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
    const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, std::shared_ptr<Collider> collider) {
    decltype(auto) object = mCurrentScene->GetInvalidObject();
    ObjectBuilder::BuildTrigger(object, event, lifeTime, eventDelay, eventCount, pos, dir, collider);

    object->SetActive(true);
    return object;
}

std::shared_ptr<GameObject>& ObjectSpawner::SpawnProjectile(ObjectTag objectType, const SimpleMath::Vector3& pos,
    const SimpleMath::Vector3& dir, GameUnits::GameUnit<GameUnits::StandardSpeed> speed, bool terraincollision) {
    decltype(auto) object = mCurrentScene->GetInvalidObject();
    ObjectBuilder::BuildProjectile(object, objectType, pos, dir, speed);

    if (true == terraincollision) {
        mCurrentScene->GetTerrainCollider().AddObjectInTerrainGroup(object);
    }

    object->SetActive(true);
    return object;
}
