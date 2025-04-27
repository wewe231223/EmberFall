#pragma once

#include "GameObject.h"

enum class InteractionType : uint8_t {
    GEM_EVENT,
};

struct InteractionInfo {
    InteractionType type{ InteractionType::GEM_EVENT };
    NetworkObjectIdType target;
};

struct GemInteraction : public InteractionInfo {
    float elapsedTime;
};

class InteractionSystem {
public:
    InteractionSystem();
    ~InteractionSystem();

public:
    bool TryAddInteractionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);
    void TryInteraction();
    void InteractionCancel(NetworkObjectIdType id1, NetworkObjectIdType id2);
    void RemoveInteractionPair(NetworkObjectIdType id1, NetworkObjectIdType id2);

    void Interact(std::shared_ptr<GameObject> obj, std::shared_ptr<GameObject> target) {
        if (nullptr == obj or nullptr == target) {
            return;
        }

        auto id1 = obj->GetId();
        auto id2 = target->GetId();
        obj->DoInteraction(target);
    }

private:
    Concurrency::concurrent_unordered_map<NetworkObjectIdType, Lock::SRWLock> mInteractionLock{ };

    Lock::SRWLock mPairLock{ };
    std::unordered_set<std::pair<NetworkObjectIdType, NetworkObjectIdType>> mInteractionPairs{ };
};
