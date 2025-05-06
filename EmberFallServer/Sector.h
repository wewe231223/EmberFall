#pragma once

#include "GameObject.h"

class ObjectManager;

class Sector {
public:
    static constexpr float DEFAULT_SECTOR_WIDTH = 200.0f;
    static constexpr float DEFAULT_SECTOR_HEIGHT = 200.0f;

public:
    Sector(uint8_t row, uint8_t col);
    ~Sector();

    Sector(const Sector& other) = delete;
    Sector& operator=(const Sector&) = delete;
    Sector(Sector&& other) noexcept;
    Sector& operator=(Sector&& other) noexcept;

public:
    Short2 GetIndex() const;
    Lock::SRWLock& GetLock();

    void TryInsert(NetworkObjectIdType id, const std::shared_ptr<ObjectManager>& objManager);
    void RemoveObject(NetworkObjectIdType id, const std::shared_ptr<ObjectManager>& objManager);
    std::vector<NetworkObjectIdType> GetTriggersInTange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager);
    std::vector<NetworkObjectIdType> GetNPCsInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager);
    std::vector<NetworkObjectIdType> GetPlayersInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager);
    std::vector<NetworkObjectIdType> GetEnvInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager);

private:
    Short2 mIndex{ };
    Lock::SRWLock mSectorLock;
    std::unordered_set<NetworkObjectIdType> mPlayers{ };
    std::unordered_set<NetworkObjectIdType> mNPCs{ };
    std::unordered_set<NetworkObjectIdType> mEnvs{ };
    std::unordered_set<NetworkObjectIdType> mTriggers{ };
    std::unordered_set<NetworkObjectIdType> mProjectiles{ };
};

class SectorSystem {
public:
    SectorSystem(std::shared_ptr<ObjectManager> objManager);
    ~SectorSystem();

public:
    Sector& GetSector(Short2 idx);
    Sector& GetSectorFromPos(const SimpleMath::Vector3& pos);
    Short2 GetSectorIdxFromPos(const SimpleMath::Vector3& pos) const;

    std::vector<Short2> GetMustCheckSectors(const SimpleMath::Vector3& pos, const float range) const;

    void AddInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos);
    void RemoveInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos);
    void AddInSector(NetworkObjectIdType id, Short2 sectorIdx);
    void RemoveInSector(NetworkObjectIdType id, Short2 sectorIdx);

    std::vector<NetworkObjectIdType> GetNearbyPlayers(const SimpleMath::Vector3& currPos, const float range);

    Short2 UpdateSectorPos(NetworkObjectIdType id, const SimpleMath::Vector3& prevPos, const SimpleMath::Vector3& currPos);
    
    void UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range);

    void UpdateEntityMove(const std::shared_ptr<GameObject>& object);

private:
    uint8_t mSectorWidth{ };
    uint8_t mSectorHeight{ };
    std::weak_ptr<ObjectManager> mObjManager{ };
    std::vector<Sector> mSectors;
};