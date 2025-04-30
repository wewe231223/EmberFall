#pragma once

#include "GameObject.h"

class ObjectManager {
public:
    static constexpr size_t MAX_USER = INVALID_SESSION_ID;
    static constexpr size_t MAX_NPC = 1000;
    static constexpr size_t MAX_PROJECTILE = 1000;
    static constexpr size_t MAX_TRIGGER_OBJ = 100;
    static constexpr size_t MAX_ENV = 5000;

    static constexpr size_t USER_ID_START = 0;                                          // 0 ~ 255
    static constexpr size_t MONSTER_ID_START = MAX_USER + 1;                            // 256 ~ 1255
    static constexpr size_t PROJECTILE_ID_START = MONSTER_ID_START + MAX_NPC;           // 1256 ~ 2255
    static constexpr size_t TRIGGER_ID_START = PROJECTILE_ID_START + MAX_PROJECTILE;    // 2256 ~ 2355
    static constexpr size_t ENV_ID_START = TRIGGER_ID_START + MAX_TRIGGER_OBJ;          // 2356 ~ 7255

    static constexpr size_t VALID_ID_MAX = ENV_ID_START + MAX_ENV;

public:
    ObjectManager();
    ~ObjectManager();

public:
    uint8_t GetGemCount() const;

    void Init();

    void LoadEnvFromFile(const std::filesystem::path& path);
    std::shared_ptr<GameObject> GetObjectFromId(NetworkObjectIdType id) const;
    std::shared_ptr<GameObject> GetPlayer(NetworkObjectIdType id) const;
    std::shared_ptr<GameObject> GetNPC(NetworkObjectIdType id) const;
    std::shared_ptr<GameObject> GetTrigger(NetworkObjectIdType id) const;
    std::shared_ptr<GameObject> GetEnv(NetworkObjectIdType id) const;
    //std::shared_ptr<GameObject> GetOther(NetworkObjectIdType id) const;

    bool InViewRange(NetworkObjectIdType id1, NetworkObjectIdType id2, const float range);

    std::shared_ptr<GameObject> SpawnObject(Packets::EntityType entity);
    std::shared_ptr<GameObject> SpawnTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir, float lifeTime);
    std::shared_ptr<GameObject> SpawnEventTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir,
        float lifeTime, std::shared_ptr<GameEvent> event, float delay, int32_t count);

    void ReleaseObject(NetworkObjectIdType id);

private:
    std::atomic_uint8_t mCorruptedGemCount{ 0 };

    std::array<std::shared_ptr<GameObject>, MAX_USER> mPlayers{ };

    std::array<std::shared_ptr<GameObject>, MAX_NPC> mNPCs{ };
    Concurrency::concurrent_queue<NetworkObjectIdType> mNPCIndices{ };
    
    std::array<std::shared_ptr<GameObject>, MAX_PROJECTILE> mProjectiles{ };
    Concurrency::concurrent_queue<NetworkObjectIdType> mProjectileIndices{ };

    std::array<std::shared_ptr<GameObject>, MAX_TRIGGER_OBJ> mTriggers{ };
    Concurrency::concurrent_queue<NetworkObjectIdType> mTriggerIndices{ };

    std::array<std::shared_ptr<GameObject>, MAX_ENV> mEnvironments{ };
    Concurrency::concurrent_queue<NetworkObjectIdType> mEnvironmentsIndices{ };
};