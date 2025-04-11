#pragma once

#include "GameObject.h"

class Sector {
public:
    static constexpr float DEFAULT_SECTOR_WIDTH = 200.0f;
    static constexpr float DEFAULT_SECTOR_HEIGHT = 200.0f;

public:
    Sector(uint8_t row, uint8_t col);
    ~Sector();

public:
    Short2 GetIndex() const;

    void TryInsert(NetworkObjectIdType id);
    std::vector<NetworkObjectIdType> GetMonstersInRange(SimpleMath::Vector3 pos, const float range) const;
    std::vector<NetworkObjectIdType> GetPlayersInRange(SimpleMath::Vector3 pos, const float range) const;

private:
    Short2 mIndex{ };
    std::vector<NetworkObjectIdType> mPlayers{ };
    std::vector<NetworkObjectIdType> mMonsters{ };
};

class SectorSystem {
public:
    SectorSystem();
    ~SectorSystem();

public:
    Sector& GetSector(Short2 idx);
    Sector& GetSectorFromPos(const SimpleMath::Vector3& pos);
    Short2 GetSectorIdxFromPos(const SimpleMath::Vector3& pos) const;
    bool GetPosInSector(Short2 idx, const SimpleMath::Vector3& pos) const;

    std::vector<Short2> GetMustCheckSectors(const SimpleMath::Vector3& pos, const float range) const;

    void AddInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos);
    void RemoveInSector(NetworkObjectIdType id);
    
    void UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range);

private:
    uint8_t mSectorWidth{ };
    uint8_t mSectorHeight{ };
    std::vector<Sector> mSectors{ };
};