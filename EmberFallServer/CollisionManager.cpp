#include "pch.h"
#include "CollisionManager.h"
#include "Sector.h"
#include "ObjectManager.h"
#include "Collider.h"

bool CollisionManager::ContainsPair(NetworkObjectIdType id1, NetworkObjectIdType id2) {
    const auto collisionPair = std::minmax(id1, id2);

    Lock::SRWLockGuard pairGuard{ Lock::SRWLockMode::SRW_SHARED, mCollisionPairLock };
    const auto iter = mCollisionPairs.find(collisionPair);
    return iter != mCollisionPairs.end();
}

bool CollisionManager::PushCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2) {
    const auto collisionPair = std::minmax(id1, id2);

    Lock::SRWLockGuard pairGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mCollisionPairLock, Lock::SRWLockTry::SRW_TRY };
    if (false == pairGuard.IsLocking()) {
        return false;
    }

    const auto [iter, result] = mCollisionPairs.insert(collisionPair);
    return result;
}

void CollisionManager::PopCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2) {
    const auto collisionPair = std::minmax(id1, id2);

    Lock::SRWLockGuard pairGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mCollisionPairLock };
    mCollisionPairs.erase(collisionPair);
}

void CollisionManager::UpdateCollision(const std::shared_ptr<GameObject>& obj) {
    if (nullptr == obj) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Error In CollisionManager: target Object is Null");
        return;
    }

    const auto pos = obj->GetPosition();
    const auto bb = obj->GetBoundingObject();
    if (nullptr == bb) {
        return;
    }

    auto bbExtents = bb->GetRadiusCircumplex();
    decltype(auto) sectors = gSectorSystem->GetMustCheckSectors(pos, bbExtents);
    const auto myId = obj->GetId();
    for (const auto sector : sectors) {
        decltype(auto) collisionCheckPlayers = std::move(gSectorSystem->GetSector(sector).GetPlayersInRange(pos, bbExtents));
        decltype(auto) collisionCheckMonsters = gSectorSystem->GetSector(sector).GetNPCsInRange(pos, bbExtents);
        decltype(auto) collisionCheckEnvs = gSectorSystem->GetSector(sector).GetEnvInRange(pos, bbExtents);

        UpdateCollisionMonster(obj, myId, collisionCheckMonsters);
        UpdateCollisionPlayer(obj, myId, collisionCheckPlayers);
        UpdateCollisionEnv(obj, myId, collisionCheckEnvs);
    }
}

void CollisionManager::UpdateCollisionMonster(const std::shared_ptr<GameObject>& obj, NetworkObjectIdType objId, const std::vector<NetworkObjectIdType>& collisionCheckMonsters) {
    for (const auto monsterId : collisionCheckMonsters) {
        if (objId == monsterId) {
            continue;
        }

        auto result = PushCollisionPair(monsterId, objId);
        if (false == result) {
            continue;
        }

        // Todo Collision Check And Resolve
        auto monster = gObjectManager->GetNPC(monsterId);
        if (nullptr == monster or false == monster->mSpec.active) {
            PopCollisionPair(monsterId, objId);
            continue;
        }

        const auto boundingObj1 = monster->GetBoundingObject();
        const auto boundingObj2 = obj->GetBoundingObject();

        auto [intersects, penetration] = boundingObj2->IsColliding(boundingObj1);
        if (intersects) {
            obj->OnCollision(monster, penetration);
        }

        PopCollisionPair(monsterId, objId);
    }
}

void CollisionManager::UpdateCollisionPlayer(const std::shared_ptr<GameObject>& obj, NetworkObjectIdType objId, const std::vector<NetworkObjectIdType>& collisionCheckPlayers) {
    for (const auto playerId : collisionCheckPlayers) {
        if (objId == playerId) {
            continue;
        }

        auto result = PushCollisionPair(playerId, objId);
        if (false == result) {
            continue;
        }

        // Todo Collision Check And Resolve
        auto player = gObjectManager->GetPlayer(playerId);
        if (nullptr == player or false == player->mSpec.active) {
            PopCollisionPair(playerId, objId);
            continue;
        }

        const auto boundingObj1 = player->GetBoundingObject();
        const auto boundingObj2 = obj->GetBoundingObject();

        auto [intersects, penetration] = boundingObj2->IsColliding(boundingObj1);
        if (intersects) {
            obj->OnCollision(player, penetration);
        }

        PopCollisionPair(playerId, objId);
    }
}

void CollisionManager::UpdateCollisionEnv(const std::shared_ptr<GameObject>& obj, NetworkObjectIdType objId, const std::vector<NetworkObjectIdType>& collisionCheckEnvs) {
    for (const auto envId : collisionCheckEnvs) {
        if (objId == envId) {
            continue;
        }

        auto result = PushCollisionPair(envId, objId);
        if (false == result) {
            continue;
        }

        // Todo Collision Check And Resolve
        auto env = gObjectManager->GetObjectFromId(envId);
        if (nullptr == env or false == env->mSpec.active) {
            PopCollisionPair(envId, objId);
            continue;
        }

        const auto boundingObj1 = env->GetBoundingObject();
        const auto boundingObj2 = obj->GetBoundingObject();

        auto [intersects, penetration] = boundingObj2->IsColliding(boundingObj1);
        if (intersects) {
            obj->OnCollision(env, penetration);
        }

        PopCollisionPair(envId, objId);
    }
}
