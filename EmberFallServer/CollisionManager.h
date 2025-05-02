#pragma once

#include "GameObject.h"

class ObjectManager;
class SectorSystem;

using CollisionPair = std::pair<NetworkObjectIdType, NetworkObjectIdType>;
using CollisionPairCont = std::unordered_set<CollisionPair>;

class CollisionManager {
public:
    bool ContainsPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    bool PushCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    void PopCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);

    void UpdateCollision(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<SectorSystem>& sectorSystem, const std::shared_ptr<ObjectManager>& objManager);
    void UpdateCollisionMonster(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckMonsters);
    void UpdateCollisionPlayer(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckPlayers);
    void UpdateCollisionEnv(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckEnvs);
    void UpdateCollisionTrigger(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckTriggers);

public:
    Lock::SRWLock mCollisionPairLock{ };
    CollisionPairCont mCollisionPairs{ };
};