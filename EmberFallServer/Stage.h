#pragma once

#include "Sector.h"
#include "ObjectManager.h"

class Stage {
public:
    Stage();
    ~Stage();

public:
    std::shared_ptr<SectorSystem> GetSector() const;
    std::shared_ptr<ObjectManager> GetObjectManager() const;
    std::shared_ptr<CollisionManager> GetCollisionManager() const;

    std::vector<NetworkObjectIdType> GetAllPlayersInThisStage();
    std::vector<NetworkObjectIdType> GetPlayersInRange(const SimpleMath::Vector3& pos, float range);
    std::vector<NetworkObjectIdType> GetNPCInRange(const SimpleMath::Vector3& pos, float range);
    std::vector<NetworkObjectIdType> GetENVInRange(const SimpleMath::Vector3& pos, float range);
    std::vector<NetworkObjectIdType> GetTriggerInTange(const SimpleMath::Vector3& pos, float range);

    void AddInSector(NetworkObjectIdType id);
    void RemoveInSector(NetworkObjectIdType id);

    void UpdateCollision(const std::shared_ptr<GameObject>& str);

private:
    std::unique_ptr<SectorSystem> mSectorSystem{ };
    std::unique_ptr<CollisionManager> mCollisionManager{ };
};