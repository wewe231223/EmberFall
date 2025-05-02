#pragma once

#include "Sector.h"
#include "ObjectManager.h"
#include "CollisionManager.h"

class Stage {
public:
    Stage(GameStage stageIdx, uint16_t roomIdx);
    ~Stage();

public:
    GameStage GetStageIdx() const;
    std::shared_ptr<SectorSystem> GetSectorSystem() const;
    std::shared_ptr<ObjectManager> GetObjectManager() const;
    std::shared_ptr<CollisionManager> GetCollisionManager() const;
    std::vector<NetworkObjectIdType> GetNearbyPlayers(const SimpleMath::Vector3& currPos, const float range);
    std::shared_ptr<GameObject> GetObjectFromId(NetworkObjectIdType id);
    std::shared_ptr<GameObject> GetPlayer(NetworkObjectIdType id);
    std::shared_ptr<GameObject> GetNPC(NetworkObjectIdType id);
    std::shared_ptr<GameObject> GetTrigger(NetworkObjectIdType id);
    //std::shared_ptr<GameObject> GetProjectile(NetworkObjectIdType id);
    std::shared_ptr<GameObject> GetEnv(NetworkObjectIdType id);

    bool InViewRange(NetworkObjectIdType id1, NetworkObjectIdType id2, const float range);

    void InitObjectManager(const std::filesystem::path& path);

    void AddInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos);
    void RemoveInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos);

    std::shared_ptr<GameObject> SpawnObject(Packets::EntityType entity);
    std::shared_ptr<GameObject> SpawnTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir, float lifeTime);
    std::shared_ptr<GameObject> SpawnEventTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir,
        float lifeTime, std::shared_ptr<GameEvent> event, float delay, int32_t count);
    void ReleaseObject(NetworkObjectIdType id);

    Short2 UpdateSectorPos(NetworkObjectIdType id, const SimpleMath::Vector3& prevPos, const SimpleMath::Vector3& currPos);
    void UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range);
    void UpdateEntityMove(const std::shared_ptr<GameObject>& object);
    void UpdateCollision(const std::shared_ptr<GameObject>& obj);

    // unsafe
    void Clear();

private:
    GameStage mStage{ };
    uint16_t mGameRoomIdx{ };

    std::shared_ptr<SectorSystem> mSectorSystem{ };
    const std::shared_ptr<ObjectManager> mObjectManager{ };
    const std::shared_ptr<CollisionManager> mCollisionManager{ };
};