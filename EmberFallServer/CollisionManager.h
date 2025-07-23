#pragma once

#include "GameObject.h"
#include "TerrainCollider.h"

class ObjectManager;
class SectorSystem;

using CollisionPair = std::pair<NetworkObjectIdType, NetworkObjectIdType>;
using CollisionPairCont = std::unordered_set<CollisionPair>;

class CollisionManager {
public:
    CollisionManager(uint16_t roomIdx);
    ~CollisionManager();

public:
    void Reset();

    bool ContainsPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    bool PushCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    void PopCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);

    void UpdateCollision(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<SectorSystem>& sectorSystem, 
        const std::shared_ptr<ObjectManager>& objManager, TerrainCollider& terrain);
    void UpdateCollisionMonster(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckMonsters);
    void UpdateCollisionPlayer(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckPlayers);
    void UpdateCollisionEnv(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckEnvs);
    void UpdateCollisionTrigger(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckTriggers);
    void UpdateCollisionProjectile(const std::shared_ptr<GameObject>& obj, const std::shared_ptr<ObjectManager>& objManager, const std::vector<NetworkObjectIdType>& collisionCheckProjectiles);

    void UpdateTerrainCollision(const std::shared_ptr<GameObject>& obj, TerrainCollider& terrain);
   
public:
    uint16_t mRoomIdx{ };
    Lock::SRWLock mCollisionPairLock{ };
    CollisionPairCont mCollisionPairs{ };
};