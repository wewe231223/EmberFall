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

    decltype(auto) sectors = gSectorSystem->GetMustCheckSectors(pos, 10.0f);
    for (const auto sector : sectors) {
        decltype(auto) collisionCheckPlayers = std::move(gSectorSystem->GetSector(sector).GetPlayersInRange(pos, 10.0f));
        decltype(auto) collisionCheckMonsters = gSectorSystem->GetSector(sector).GetMonstersInRange(pos, 10.0f);

        for (const auto playerId : collisionCheckPlayers) {
            auto result = PushCollisionPair(playerId, obj->GetId());
            if (false == result) {
                continue;
            }

            // Todo Collision Check And Resolve
            auto player = gObjectManager->GetPlayer(playerId);
            const auto boundingObj1 = player->GetBoundingObject();
            const auto boundingObj2 = obj->GetBoundingObject();

            auto [intersects, penetration] = boundingObj2->IsColliding(boundingObj1);
            obj->OnCollision(player, penetration);

            PopCollisionPair(playerId, obj->GetId());
        }

        for (const auto monsterId : collisionCheckMonsters) {
            auto result = PushCollisionPair(monsterId, obj->GetId());
            if (false == result) {
                continue;
            }

            // Todo Collision Check And Resolve
            auto monster = gObjectManager->GetPlayer(monsterId);
            const auto boundingObj1 = monster->GetBoundingObject();
            const auto boundingObj2 = obj->GetBoundingObject();

            auto [intersects, penetration] = boundingObj2->IsColliding(boundingObj1);
            obj->OnCollision(monster, penetration);

            PopCollisionPair(monsterId, obj->GetId());
        }
    }
}
