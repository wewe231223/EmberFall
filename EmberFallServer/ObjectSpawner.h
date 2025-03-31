#pragma once

#include "GameObject.h"
#include "ServerGameScene.h"

class ObjectSpawner {
public:
    ObjectSpawner();
    ~ObjectSpawner();

public:
    void SetCurrentScene(std::shared_ptr<class IServerGameScene> gameScene);

    std::shared_ptr<GameObject>& SpawnObject(ObjectTag objectType, bool terrainCollision = true);
    std::shared_ptr<GameObject>& SpawnTrigger(float lifeTime, const SimpleMath::Vector3& pos, const SimpleMath::Vector3 extents);
    std::shared_ptr<GameObject>& SpawnTrigger(float lifeTime, const SimpleMath::Vector3& pos, std::shared_ptr<Collider> colldier);

    std::shared_ptr<GameObject>& SpawnEventTrigger(std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
        const SimpleMath::Vector3& pos, const SimpleMath::Vector3& size, const SimpleMath::Vector3& dir);

    std::shared_ptr<GameObject>& SpawnEventTrigger(std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
        const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, std::shared_ptr<class Collider> collider);

    std::shared_ptr<GameObject>& SpawnProjectile(ObjectTag objectType, const SimpleMath::Vector3& pos,
        const SimpleMath::Vector3& dir, GameUnits::GameUnit<GameUnits::StandardSpeed> speed, bool terraincollision = true);

private:
    std::shared_ptr<class IServerGameScene> mCurrentScene{ };
};

