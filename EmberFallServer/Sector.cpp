#include "pch.h"
#include "Sector.h"
#include "ObjectManager.h"
#include "PlayerScript.h"

Sector::Sector(uint8_t row, uint8_t col) 
    : mIndex{ col, row } {
}

Sector::~Sector() {
    mMonsters.clear();
    mPlayers.clear();
}

Short2 Sector::GetIndex() const {
    return mIndex;
}

void Sector::TryInsert(NetworkObjectIdType id) {
    auto tag = gObjectManager->GetObjectFromId(id)->GetTag();

    switch (tag) {
    case ObjectTag::PLAYER:
    case ObjectTag::BOSSPLAYER:
    {
        mPlayers.emplace_back(id);
        break;
    }

    case ObjectTag::MONSTER:
    {
        mMonsters.emplace_back(id);
        break;
    }

    default:
        break;
    }
}

std::vector<NetworkObjectIdType> Sector::GetMonstersInRange(SimpleMath::Vector3 pos, const float range) const {
    std::vector<NetworkObjectIdType> inRangeMonsters{ };
    for (const auto& monsterId : mMonsters) {
        decltype(auto) monsterPos = gObjectManager->GetObjectFromId(monsterId)->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, monsterPos);
        inRangeMonsters.emplace_back(monsterId);
    }

    return inRangeMonsters;
}

std::vector<NetworkObjectIdType> Sector::GetPlayersInRange(SimpleMath::Vector3 pos, const float range) const {
    std::vector<NetworkObjectIdType> inRangePlayer{ };
    for (const auto& playerId : mPlayers) {
        decltype(auto) playerPos = gObjectManager->GetObjectFromId(playerId)->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, playerPos);
        inRangePlayer.emplace_back(playerId);
    }

    return inRangePlayer;
}

SectorSystem::SectorSystem() { 
    auto mapHeight = 1000.0f;
    auto mapWidth = 1000.0f;
    
    const auto rows = static_cast<uint8_t>(mapHeight / Sector::DEFAULT_SECTOR_HEIGHT);
    const auto cols = static_cast<uint8_t>(mapWidth / Sector::DEFAULT_SECTOR_WIDTH);
    mSectorWidth = rows;
    mSectorHeight = cols;

    mSectors.reserve(rows * cols);
    for (uint8_t sectorRow = 0; sectorRow < rows; ++sectorRow) {
        for (uint8_t sectorCol = 0; sectorCol < cols; ++sectorCol) {
            mSectors.emplace_back(sectorCol, sectorRow);
        }
    }
}

SectorSystem::~SectorSystem() { }

Sector& SectorSystem::GetSector(Short2 idx) {
    if (idx.x < 0 or idx.y < 0 or
        idx.x >= mSectorWidth or idx.y >= mSectorHeight) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Sector Access: Try Access [{}][{}]", idx.x, idx.y);
        Crash("Bad Memory Access");
    }

    return mSectors[idx.y * mSectorWidth + idx.x];
}

Sector& SectorSystem::GetSectorFromPos(const SimpleMath::Vector3& pos) {
    const Short2 idx = { static_cast<int16_t>(pos.x / Sector::DEFAULT_SECTOR_WIDTH), static_cast<int16_t>(pos.z / Sector::DEFAULT_SECTOR_HEIGHT) };
    return GetSector(idx);
}

Short2 SectorSystem::GetSectorIdxFromPos(const SimpleMath::Vector3& pos) const {
    const Short2 idx = { static_cast<int16_t>(pos.x / Sector::DEFAULT_SECTOR_WIDTH), static_cast<int16_t>(pos.z / Sector::DEFAULT_SECTOR_HEIGHT) };
    if (idx.x < 0 or idx.y < 0 or
        idx.x >= mSectorWidth or idx.y >= mSectorHeight) {
        return Short2{ -1, -1 };
    }

    return idx;
}

bool SectorSystem::GetPosInSector(Short2 idx, const SimpleMath::Vector3& pos) const {

    return false;
}

std::vector<Short2> SectorSystem::GetMustCheckSectors(const SimpleMath::Vector3& pos, const float range) const {
    const auto invalidIdx = Short2{ -1, -1 };
    std::vector<Short2> checkSector{ };

    const auto checkIdx = GetSectorIdxFromPos(pos);
    if (checkIdx == invalidIdx) {
        return {};
    }

    checkSector.emplace_back(checkIdx);

    const auto checkForward = GetSectorIdxFromPos(pos - (SimpleMath::Vector3::Forward * range));
    const auto checkBackward= GetSectorIdxFromPos(pos - (SimpleMath::Vector3::Backward * range));
    const auto checkLeft = GetSectorIdxFromPos(pos - (SimpleMath::Vector3::Left * range));
    const auto checkRight = GetSectorIdxFromPos(pos - (SimpleMath::Vector3::Right * range));


    bool forward{ };
    bool backward{ };
    bool left{ };
    bool right{ };

    if (checkForward != invalidIdx and checkForward != checkIdx) {
        forward = true;
        checkSector.emplace_back(checkForward);
    }

    if (checkBackward != invalidIdx and checkBackward != checkIdx) {
        backward = true;
        checkSector.emplace_back(checkBackward);
    }

    if (checkLeft != invalidIdx and checkLeft != checkIdx) {
        left = true;
        checkSector.emplace_back(checkLeft);
    }

    if (checkRight != invalidIdx and checkRight != checkIdx) {
        right = true;
        checkSector.emplace_back(checkRight);
    }

    if (forward and left) {
        checkSector.emplace_back(checkIdx.x - 1, checkIdx.y + 1);
    }

    if (forward and right) {
        checkSector.emplace_back(checkIdx.x + 1, checkIdx.y + 1);
    }

    if (backward and left) {
        checkSector.emplace_back(checkIdx.x - 1, checkIdx.y - 1);
    }

    if (backward and right) {
        checkSector.emplace_back(checkIdx.x + 1, checkIdx.y - 1);
    }
    
    return checkSector;
}

void SectorSystem::AddInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos) { 
    GetSectorFromPos(pos).TryInsert(id);
}

void SectorSystem::RemoveInSector(NetworkObjectIdType id) { }

void SectorSystem::UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range) {
    const auto playerScript = player->GetScript<PlayerScript>();
    if (nullptr == playerScript) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "In Sector System: PlayerScript Is Null");
        return;
    }

    std::vector<Short2> checkSectors = std::move(GetMustCheckSectors(pos, range));
    for (const auto idx : checkSectors) {
        const std::vector<NetworkObjectIdType> monsters = std::move(GetSector(idx).GetMonstersInRange(pos, range));
        const std::vector<NetworkObjectIdType> players = std::move(GetSector(idx).GetPlayersInRange(pos, range));

        playerScript->UpdateViewListNPC(monsters);
        playerScript->UpdateViewListPlayer(players);
    }
}
