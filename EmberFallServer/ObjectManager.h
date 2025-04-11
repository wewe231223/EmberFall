#pragma once

#include "GameObject.h"

class ObjectManager {
public:
    static constexpr size_t MAX_USER = INVALID_SESSION_ID;
    static constexpr size_t MAX_MONSTER = 1000;
    static constexpr size_t MAX_PROJECTILE = 1000;
    static constexpr size_t MAX_ENV = 5000;

    static constexpr size_t USER_ID_START = 0;                                      // 0 ~ 255
    static constexpr size_t MONSTER_ID_START = MAX_USER + 1;                        // 256 ~ 1255
    static constexpr size_t PROJECTILE_ID_START = MONSTER_ID_START + MAX_MONSTER;   // 1256 ~ 2255
    static constexpr size_t ENV_ID_START = PROJECTILE_ID_START + MAX_PROJECTILE;    // 2255 ~ 7255

    static constexpr size_t VALID_ID_MAX = ENV_ID_START + MAX_ENV;

public:
    ObjectManager();
    ~ObjectManager();

public:
    void LoadEnvFromFile();
    std::shared_ptr<GameObject> GetObjectFromId(NetworkObjectIdType id) const;

    bool InViewRange(NetworkObjectIdType id1, NetworkObjectIdType id2, const float range);

private:
    std::array<std::shared_ptr<GameObject>, MAX_USER> mPlayers{ };
    std::array<std::shared_ptr<GameObject>, MAX_MONSTER> mMonsters{ };
    std::array<std::shared_ptr<GameObject>, MAX_ENV> mEnvironments{ };

    Concurrency::concurrent_queue<NetworkObjectIdType> mFreeIndices{ };
};