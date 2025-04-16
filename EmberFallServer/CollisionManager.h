#pragma once

#include "GameObject.h"

using CollisionPair = std::pair<NetworkObjectIdType, NetworkObjectIdType>;
using CollisionPairCont = std::unordered_set<CollisionPair>;

class CollisionManager {
public:
    bool ContainsPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    bool PushCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    void PopCollisionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);

    void UpdateCollision(const std::shared_ptr<GameObject>& obj);

public:
    Lock::SRWLock mCollisionPairLock{ };
    CollisionPairCont mCollisionPairs{ };
};