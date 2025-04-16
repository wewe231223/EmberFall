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

Sector::Sector(Sector&& other) noexcept 
    : mPlayers{ std::move(other.mPlayers) }, mMonsters{ std::move(other.mMonsters) } {
}

Sector& Sector::operator=(Sector&& other) noexcept {
    mPlayers = std::move(other.mPlayers); 
    mMonsters = std::move(other.mMonsters);
    return *this;
}

Short2 Sector::GetIndex() const {
    return mIndex;
}

Lock::SRWLock& Sector::GetLock() {
    return mSectorLock;
}

void Sector::TryInsert(NetworkObjectIdType id) {
    auto tag = gObjectManager->GetObjectFromId(id)->GetTag();

    switch (tag) {
    case ObjectTag::PLAYER:
    case ObjectTag::BOSSPLAYER:
    {
        mPlayers.insert(id);
        break;
    }

    case ObjectTag::MONSTER:
    {
        mMonsters.insert(id);
        break;
    }

    default:
        break;
    }
}

void Sector::RemoveObject(NetworkObjectIdType id) {
    auto tag = gObjectManager->GetObjectFromId(id)->GetTag();

    switch (tag) {
    case ObjectTag::PLAYER:
    case ObjectTag::BOSSPLAYER:
    {
        if (false == mPlayers.contains(id)) {
            return;
        }

        mPlayers.erase(id);
        break;
    }

    case ObjectTag::MONSTER:
    {
        if (false == mMonsters.contains(id)) {
            return;
        }

        mMonsters.erase(id);
        break;
    }

    default:
        break;
    }
}

std::vector<NetworkObjectIdType> Sector::GetMonstersInRange(SimpleMath::Vector3 pos, const float range) {
    Lock::SRWLockGuard guard{ Lock::SRWLockMode::SRW_SHARED, mSectorLock };
    std::vector<NetworkObjectIdType> inRangeMonsters{ };
    for (const auto monsterId : mMonsters) {
        decltype(auto) monsterPos = gObjectManager->GetObjectFromId(monsterId)->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, monsterPos);
        inRangeMonsters.emplace_back(monsterId);
    }

    return inRangeMonsters;
}

std::vector<NetworkObjectIdType> Sector::GetPlayersInRange(SimpleMath::Vector3 pos, const float range) {
    Lock::SRWLockGuard guard{ Lock::SRWLockMode::SRW_SHARED, mSectorLock };
    std::vector<NetworkObjectIdType> inRangePlayer{ };
    for (const auto playerId : mPlayers) {
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

    return std::ref(mSectors[idx.y * mSectorWidth + idx.x]);
}

Sector& SectorSystem::GetSectorFromPos(const SimpleMath::Vector3& pos) {
    const Short2 idx = { 
        static_cast<int16_t>((pos.x + 500.0f) / Sector::DEFAULT_SECTOR_WIDTH),
        static_cast<int16_t>((pos.z + 500.0f) / Sector::DEFAULT_SECTOR_HEIGHT)
    };
    return GetSector(idx);
}

Short2 SectorSystem::GetSectorIdxFromPos(const SimpleMath::Vector3& pos) const {
    const Short2 idx = { 
        static_cast<int16_t>((pos.x + 500.0f) / Sector::DEFAULT_SECTOR_WIDTH),
        static_cast<int16_t>((pos.z + 500.0f) / Sector::DEFAULT_SECTOR_HEIGHT)
    };
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
    const auto checkBackward = GetSectorIdxFromPos(pos - (SimpleMath::Vector3::Backward * range));
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
    const auto idx = GetSectorIdxFromPos(pos);
    if (Short2{ -1, -1 } == idx) {
        return;
    }

    decltype(auto) sector = GetSector(idx);
    Lock::SRWLockGuard sectorGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, sector.GetLock() };
    sector.TryInsert(id);
}

void SectorSystem::RemoveInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos) { 
    const auto idx = GetSectorIdxFromPos(pos);
    if (Short2{ -1, -1 } == idx) {
        return;
    }

    decltype(auto) sector = GetSector(idx);
    Lock::SRWLockGuard sectorGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, sector.GetLock() };
    sector.RemoveObject(id);
}

std::vector<NetworkObjectIdType> SectorSystem::GetNearbyPlayers(const SimpleMath::Vector3& currPos, const float range) {
    std::vector<Short2> checkSectors = std::move(GetMustCheckSectors(currPos, range));
    std::vector<NetworkObjectIdType> nearbyPlayers;
    for (const auto idx : checkSectors) {
        decltype(auto) sector = GetSector(idx);

        const std::vector<NetworkObjectIdType> players = std::move(sector.GetPlayersInRange(currPos, range));
        nearbyPlayers.insert(nearbyPlayers.end(), players.begin(), players.end());
    }

    return nearbyPlayers;
}

void SectorSystem::UpdateSectorPos(NetworkObjectIdType id, const SimpleMath::Vector3& prevPos, const SimpleMath::Vector3& currPos) {
    if (MathUtil::IsEqualVector(currPos, prevPos)) {
        return;
    }

    RemoveInSector(id, prevPos);
    AddInSector(id, currPos);
}

void SectorSystem::UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range) {
    const auto playerScript = player->GetScript<PlayerScript>();
    if (nullptr == playerScript) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "In Sector System: PlayerScript Is Null");
        return;
    }

    const auto id = player->GetId();
    const auto currPos = player->GetPosition();
    const auto prevPos = player->GetPrevPosition();
    UpdateSectorPos(id, prevPos, currPos);

    std::vector<Short2> checkSectors = std::move(GetMustCheckSectors(pos, range));
    for (const auto idx : checkSectors) {
        decltype(auto) sector = GetSector(idx);

        const std::vector<NetworkObjectIdType> monsters = std::move(sector.GetMonstersInRange(pos, range));
        const std::vector<NetworkObjectIdType> players = std::move(sector.GetPlayersInRange(pos, range));

        playerScript->UpdateViewListNPC(monsters);
        playerScript->UpdateViewListPlayer(players);
    }
}

void SectorSystem::UpdateEntityMove(const std::shared_ptr<GameObject>& object) {
    const auto id = object->GetId();
    const auto currPos = object->GetPosition();
    const auto prevPos = object->GetPrevPosition();
    UpdateSectorPos(id, prevPos, currPos);
    auto currSector = GetSectorIdxFromPos(currPos);

    const auto yaw = object->GetEulerRotation().y;
    const auto dir = object->GetMoveDir();
    const auto speed = object->GetSpeed();

    const float range = 100.0f;
    const std::vector<NetworkObjectIdType> nearbyPlayers = std::move(GetNearbyPlayers(currPos, range));
    for (const auto playerId : nearbyPlayers) {
        auto playerObj = gObjectManager->GetObjectFromId(playerId);
        auto viewRange = playerObj->GetScript<PlayerScript>()->GetViewList().mViewRange.Count();
        if (false == gObjectManager->InViewRange(playerId, id, viewRange)) {
            continue;
        }

        auto packetMove = FbsPacketFactory::ObjectMoveSC(id, yaw, currPos, dir, speed);
        gServerCore->Send(static_cast<SessionIdType>(playerId), packetMove);
    }
}