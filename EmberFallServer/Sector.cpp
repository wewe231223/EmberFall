#include "pch.h"
#include "Sector.h"
#include "ObjectManager.h"
#include "PlayerScript.h"
#include "GameSession.h"

Sector::Sector(uint8_t row, uint8_t col) 
    : mIndex{ col, row } {
}

Sector::~Sector() {
    mNPCs.clear();
    mPlayers.clear();
}

Sector::Sector(Sector&& other) noexcept 
    : mPlayers{ std::move(other.mPlayers) }, mNPCs{ std::move(other.mNPCs) } {
}

Sector& Sector::operator=(Sector&& other) noexcept {
    mPlayers = std::move(other.mPlayers); 
    mNPCs = std::move(other.mNPCs);
    return *this;
}

Short2 Sector::GetIndex() const {
    return mIndex;
}

Lock::SRWLock& Sector::GetLock() {
    return mSectorLock;
}

void Sector::TryInsert(NetworkObjectIdType id, const std::shared_ptr<ObjectManager>& objManager) {
    auto obj = objManager->GetObjectFromId(id);
    if (nullptr == obj) {
        return;
    }

    auto tag = obj->GetTag();

    switch (tag) {
    case ObjectTag::PLAYER:
    case ObjectTag::BOSSPLAYER:
    {
        mPlayers.insert(id);
        break;
    }

    case ObjectTag::MONSTER:
    case ObjectTag::CORRUPTED_GEM:
    case ObjectTag::ITEM:
    {
        mNPCs.insert(id);
        break;
    }

    case ObjectTag::ENV:
    {
        mEnvs.insert(id);
        break;
    }

    case ObjectTag::TRIGGER:
    {
        mTriggers.insert(id);
        break;
    }

    default:
        break;
    }
}

void Sector::RemoveObject(NetworkObjectIdType id, const std::shared_ptr<ObjectManager>& objManager) {
    auto tag = objManager->GetObjectFromId(id)->GetTag();

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
    case ObjectTag::CORRUPTED_GEM:
    case ObjectTag::ITEM:
    {
        if (false == mNPCs.contains(id)) {
            return;
        }

        mNPCs.erase(id);
        break;
    }

    case ObjectTag::ENV: 
    {
        if (false == mEnvs.contains(id)) {
            return;
        }

        mEnvs.erase(id);
        break;
    }

    case ObjectTag::TRIGGER:
    {
        if (false == mTriggers.contains(id)) {
            return;
        }

        mTriggers.erase(id);
        break;
    }

    default:
        break;
    }
}

std::vector<NetworkObjectIdType> Sector::GetTriggersInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager) {
    Lock::SRWLockGuard guard{ Lock::SRWLockMode::SRW_SHARED, mSectorLock };
    std::vector<NetworkObjectIdType> inRangeTriggers{ };
    for (const auto triggerId : mTriggers) {
        auto trigger = objManager->GetTrigger(triggerId);
        if (nullptr == trigger) {
            continue;
        }

        decltype(auto) triggerPos = trigger->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, triggerPos);
        if (dist <= MathUtil::Square(range)) {
            inRangeTriggers.emplace_back(triggerId);
        }
    }

    return inRangeTriggers;
}

std::vector<NetworkObjectIdType> Sector::GetNPCsInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager) {
    Lock::SRWLockGuard guard{ Lock::SRWLockMode::SRW_SHARED, mSectorLock };
    std::vector<NetworkObjectIdType> inRangeNPCs{ };
    for (const auto npcId : mNPCs) {
        auto npc = objManager->GetNPC(npcId);
        if (nullptr == npc) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "NPC {} is null", npcId);
            continue;
        }

        decltype(auto) npcPos = npc->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, npcPos);
        if (dist <= MathUtil::Square(range)) {
            inRangeNPCs.emplace_back(npcId);
        }
    }

    return inRangeNPCs;
}

std::vector<NetworkObjectIdType> Sector::GetPlayersInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager) {
    Lock::SRWLockGuard guard{ Lock::SRWLockMode::SRW_SHARED, mSectorLock };
    std::vector<NetworkObjectIdType> inRangePlayer{ };
    for (const auto playerId : mPlayers) {
        auto player = objManager->GetPlayer(playerId);
        if (nullptr == player) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player {} is null", playerId);
            continue;
        }

        decltype(auto) playerPos = player->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, playerPos);
        if (dist <= MathUtil::Square(range)) {
            inRangePlayer.emplace_back(playerId);
        }
    }

    return inRangePlayer;
}

std::vector<NetworkObjectIdType> Sector::GetEnvInRange(SimpleMath::Vector3 pos, const float range, const std::shared_ptr<ObjectManager>& objManager) {
    Lock::SRWLockGuard guard{ Lock::SRWLockMode::SRW_SHARED, mSectorLock };
    std::vector<NetworkObjectIdType> inRangeEnv{ };
    for (const auto envId : mEnvs) {
        auto env = objManager->GetEnv(envId);
        if (nullptr == env) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "ENV {} is null", envId);
            continue;
        }

        decltype(auto) envPos = env->GetPosition();

        auto dist = SimpleMath::Vector3::DistanceSquared(pos, envPos);
        if (dist <= MathUtil::Square(range)) {
            inRangeEnv.emplace_back(envId);
        }
    }

    return inRangeEnv;
}

SectorSystem::SectorSystem(std::shared_ptr<class ObjectManager> objManager) 
    : mObjManager{ objManager } {
    auto mapHeight = 2000.0f;
    auto mapWidth = 2000.0f;

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
        static_cast<int16_t>((pos.x + 1000.0f) / Sector::DEFAULT_SECTOR_WIDTH),
        static_cast<int16_t>((pos.z + 1000.0f) / Sector::DEFAULT_SECTOR_HEIGHT)
    };
    return GetSector(idx);
}

Short2 SectorSystem::GetSectorIdxFromPos(const SimpleMath::Vector3& pos) const {
    const Short2 idx = { 
        static_cast<int16_t>((pos.x + 1000.0f) / Sector::DEFAULT_SECTOR_WIDTH),
        static_cast<int16_t>((pos.z + 1000.0f) / Sector::DEFAULT_SECTOR_HEIGHT)
    };
    if (idx.x < 0 or idx.y < 0 or
        idx.x >= mSectorWidth or idx.y >= mSectorHeight) {
        Crash("Bad Sector Access");
        return Short2{ -1, -1 };
    }

    return idx;
}

std::vector<Short2> SectorSystem::GetMustCheckSectors(const SimpleMath::Vector3& pos, const float range) const {
    const auto invalidIdx = Short2{ -1, -1 };
    std::vector<Short2> checkSector{ };

    const auto checkIdx = GetSectorIdxFromPos(pos);
    if (checkIdx == invalidIdx) {
        return {};
    }

    checkSector.emplace_back(checkIdx);

    const auto checkForward = GetSectorIdxFromPos(pos + (SimpleMath::Vector3::Forward * range));
    const auto checkBackward = GetSectorIdxFromPos(pos + (SimpleMath::Vector3::Backward * range));
    const auto checkLeft = GetSectorIdxFromPos(pos + (SimpleMath::Vector3::Left * range));
    const auto checkRight = GetSectorIdxFromPos(pos + (SimpleMath::Vector3::Right * range));

    bool forward{ false };
    bool backward{ false };
    bool left{ false };
    bool right{ false };

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
        checkSector.emplace_back(checkIdx.x - 1, checkIdx.y - 1);
    }

    if (forward and right) {
        checkSector.emplace_back(checkIdx.x + 1, checkIdx.y - 1);
    }

    if (backward and left) {
        checkSector.emplace_back(checkIdx.x - 1, checkIdx.y + 1);
    }

    if (backward and right) {
        checkSector.emplace_back(checkIdx.x + 1, checkIdx.y + 1);
    }
    
    return checkSector;
}

void SectorSystem::AddInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos) { 
    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return;
    }

    const auto idx = GetSectorIdxFromPos(pos);
    if (Short2{ -1, -1 } == idx) {
        return;
    }

    decltype(auto) sector = GetSector(idx);
    Lock::SRWLockGuard sectorGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, sector.GetLock() };
    sector.TryInsert(id, objManager);
}

void SectorSystem::RemoveInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos) { 
    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return;
    }

    const auto idx = GetSectorIdxFromPos(pos);
    if (Short2{ -1, -1 } == idx) {
        return;
    }

    decltype(auto) sector = GetSector(idx);
    Lock::SRWLockGuard sectorGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, sector.GetLock() };
    sector.RemoveObject(id, objManager);
}

void SectorSystem::AddInSector(NetworkObjectIdType id, Short2 sectorIdx) {
    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return;
    }

    decltype(auto) sector = GetSector(sectorIdx);
    Lock::SRWLockGuard sectorGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, sector.GetLock() };
    sector.TryInsert(id, objManager);
}

void SectorSystem::RemoveInSector(NetworkObjectIdType id, Short2 sectorIdx) {
    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return;
    }

    decltype(auto) sector = GetSector(sectorIdx);
    Lock::SRWLockGuard sectorGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, sector.GetLock() };
    sector.RemoveObject(id, objManager);
}

void SectorSystem::ChangeSector(NetworkObjectIdType id, Short2 prevIdx, Short2 currIdx) {
    auto objManager = mObjManager.lock();
    if (nullptr == objManager or prevIdx == currIdx) {
        return;
    }

    auto object = objManager->GetObjectFromId(id);
    if (nullptr == object) {
        return;
    }
    auto tag = object->GetTag();
    if (tag == ObjectTag::PLAYER) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Change Sector From: {}, {}, To: {}, {}", prevIdx.x, prevIdx.y, currIdx.x, currIdx.y);
    }

    decltype(auto) prevSector = GetSector(prevIdx);
    decltype(auto) currSector = GetSector(currIdx);

    {
        // Locking
        Lock::ScopedSRWLock sectorGuard{
            prevSector.GetLock(), Lock::SRWLockMode::SRW_EXCLUSIVE,
            currSector.GetLock(),Lock::SRWLockMode::SRW_EXCLUSIVE
        };

        prevSector.RemoveObject(id, objManager);
        currSector.TryInsert(id, objManager);
    }
}

std::vector<NetworkObjectIdType> SectorSystem::GetNearbyPlayers(const SimpleMath::Vector3& currPos, const float range) {
    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return { };
    }

    std::vector<Short2> checkSectors = std::move(GetMustCheckSectors(currPos, range));
    std::vector<NetworkObjectIdType> nearbyPlayers;
    for (const auto idx : checkSectors) {
        decltype(auto) sector = GetSector(idx);

        const std::vector<NetworkObjectIdType> players = std::move(sector.GetPlayersInRange(currPos, range, objManager));
        nearbyPlayers.insert(nearbyPlayers.end(), players.begin(), players.end());
    }

    return nearbyPlayers;
}

Short2 SectorSystem::UpdateSectorPos(NetworkObjectIdType id, const SimpleMath::Vector3& prevPos, const SimpleMath::Vector3& currPos) {
    const auto prevIdx = GetSectorIdxFromPos(prevPos);
    const auto currIdx = GetSectorIdxFromPos(currPos);
    if (prevIdx == currIdx) {
        return currIdx;
    }

    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return { };
    }

    auto obj = objManager->GetObjectFromId(id);
    if (nullptr == obj) {
        return currIdx;
    }

    ChangeSector(obj->GetId(), prevIdx, currIdx);
    return currIdx;
}

void SectorSystem::UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range) {
    const auto playerScript = player->GetScript<PlayerScript>();
    if (nullptr == playerScript) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "In Sector System: PlayerScript Is Null");
        return;
    }

    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return;
    }

    static SysClock::time_point logT{ SysClock::now() };
    if (player->GetId() == 0) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(SysClock::now() - logT).count() > 1000) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Update View List, Range: {}", range);
        }
    }

    const auto id = player->GetId();
    const auto currPos = player->GetPosition();
    const auto prevPos = player->GetPrevPosition();
    auto currIdx = UpdateSectorPos(id, prevPos, currPos);

    std::vector<Short2> checkSectors = std::move(GetMustCheckSectors(pos, range));
    //test
    if (std::chrono::duration_cast<std::chrono::milliseconds>(SysClock::now() - logT).count() > 1000) {
        for (auto sector : checkSectors) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "MustCheckSector: {}, {}", sector.x, sector.y);
        }
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "\n\n\n");
        logT = SysClock::now();
    }

    std::vector<NetworkObjectIdType> inViewRangeMonsters{ };
    std::vector<NetworkObjectIdType> inViewRangePlayers{ };
    for (const auto idx : checkSectors) {
        decltype(auto) sector = GetSector(idx);

        const std::vector<NetworkObjectIdType> monsters = std::move(sector.GetNPCsInRange(pos, range, objManager));
        const std::vector<NetworkObjectIdType> players = std::move(sector.GetPlayersInRange(pos, range, objManager));

        inViewRangeMonsters.insert(inViewRangeMonsters.end(), monsters.begin(), monsters.end());
        inViewRangePlayers.insert(inViewRangePlayers.end(), players.begin(), players.end());
    }
    playerScript->UpdateViewList(inViewRangeMonsters, inViewRangePlayers);
}

void SectorSystem::UpdateEntityMove(const std::shared_ptr<GameObject>& object) {
    auto objManager = mObjManager.lock();
    if (nullptr == objManager) {
        return;
    }

    const auto id = object->GetId();
    const auto currPos = object->GetPosition();
    const auto prevPos = object->GetPrevPosition();
    const auto speed = object->GetSpeed();

    auto currSector = UpdateSectorPos(id, prevPos, currPos);

    const auto yaw = object->GetEulerRotation().y;
    const auto dir = object->GetMoveDir();

    const float range = GameProtocol::Logic::PLAYER_VIEW_RANGE;
    const std::vector<NetworkObjectIdType> nearbyPlayers = std::move(GetNearbyPlayers(currPos, range));
    OverlappedSend* sendPacket{ nullptr };
    if (nearbyPlayers.empty()) {
        while (true == object->GetSendBuf().try_pop(sendPacket)); // clear
        return;
    }

    while (true == object->GetSendBuf().try_pop(sendPacket)) {
        if (nullptr == sendPacket) {
            break;
        }

        for (const auto playerId : nearbyPlayers) {
            auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(static_cast<SessionIdType>(playerId)));
            if (nullptr == session or SESSION_INGAME != session->GetSessionState()) {
                continue;
            }

            auto playerObj = session->GetUserObject();
            if (nullptr == playerObj) {
                continue;
            }

            auto playerScript = playerObj->GetScript<PlayerScript>();
            if (nullptr == playerScript) {
                continue;
            }

            auto packet = FbsPacketFactory::ClonePacket(sendPacket);
            session->RegisterSend(packet);
        }

        FbsPacketFactory::ReleasePacketBuf(sendPacket);
    }
}