#include "pch.h"
#include "GameRoom.h"
#include "ServerFrame.h"

GameRoom::GameRoom(uint16_t roomIdx)
    : mRoomIdx{ roomIdx }, mGameRoomState{ GAME_ROOM_STATE_LOBBY }, mStage{ GameStage::STAGE1, roomIdx } { 
    for (uint8_t i = 0; i < MAX_PLAYER_IN_GAME_ROOM; ++i) {
        mSessionSlotIndices.push(i);
    }
}

GameRoom::~GameRoom() { }

bool GameRoom::IsEveryPlayerReady() const {
    return mPlayerCount == mReadyPlayerCount;
}

bool GameRoom::IsMaxSession() const {
    Lock::SRWLockGuard sessionGaurd{ Lock::SRWLockMode::SRW_SHARED, mSessionLock };
    if (mSessionsInRoom.size() >= MAX_PLAYER_IN_GAME_ROOM) {
        return true;
    }

    return false;
}

uint8_t GameRoom::GetGameRoomState() const {
    return mGameRoomState;
}

Stage& GameRoom::GetStage() {
    return mStage;
}

Lock::SRWLock& GameRoom::GetSessionLock() {
    return mSessionLock;
}

SessionListInRoom& GameRoom::GetSessions() {
    return mSessionsInRoom;
}

uint8_t GameRoom::TryInsertInRoom(SessionIdType sessionId) {
    Lock::SRWLockGuard sessionGaurd{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionLock };
    if (GAME_ROOM_STATE_INGAME == mGameRoomState) {
        return GameRoomError::ERROR_ROOM_STATE_IS_INGAME;
    }

    if (mSessionsInRoom.contains(sessionId)) {
        return GameRoomError::ERROR_ROOM_STATE_IS_INGAME;
    }

    if (mSessionsInRoom.size() >= MAX_PLAYER_IN_GAME_ROOM) {
        return GameRoomError::ERROR_MAX_SESSION_IN_ONE_ROOM;
    }

    uint8_t slotIndex;
    if (false == mSessionSlotIndices.try_pop(slotIndex)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Slot Index is exhausted!!");
        ::exit(-1);
    }

    auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
    if (nullptr == session) {
        return GameRoomError::ERROR_SESSION_EXISTS_IN_THIS_ROOM;
    }

    ++mPlayerCount;
    session->SetSlotIndex(slotIndex);
    session->SetName(std::format("PID {}", slotIndex));
    mSessionsInRoom.insert(sessionId);

    return GameRoomError::SUCCESS_INSERT_SESSION_IN_ROOM;
}

uint8_t GameRoom::RemovePlayer(SessionIdType id, bool lastReadyState, uint8_t lastSlotIndex) {
    Lock::SRWLockGuard sessionGaurd{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionLock };
    if (not mSessionsInRoom.contains(id)) {
        return GameRoomError::ERROR_SESSION_NOT_EXISTS_IN_THIS_ROOM;
    }

    if (true == lastReadyState) {
        --mReadyPlayerCount;
    }

    --mPlayerCount;

    mSessionsInRoom.erase(id);
    mSessionSlotIndices.push(lastSlotIndex);
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session [{}] erased in GameRoom [{}], last slot: [{}]", id, mRoomIdx, lastSlotIndex);

    return GameRoomError::SUCCESS_REMOVE_SESSION_IN_ROOM;
}

bool GameRoom::ChangeRolePlayer(SessionIdType id, Packets::PlayerRole role) {
    mSessionLock.ReadLock();
    if (not mSessionsInRoom.contains(id)) {
        return false;
    }
    mSessionLock.ReadUnlock();

    auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(id));
    if (nullptr == session) {
        return false;
    }

    auto oldRole = session->GetPlayerRole();
    if (oldRole != role and Packets::PlayerRole_BOSS == oldRole) {
        uint8_t expectedBossPlayerCount = 1;
        mBossPlayerCount.compare_exchange_strong(expectedBossPlayerCount, 0);

        session->ChangeRole(role);
        return true;
    }

    if (role != Packets::PlayerRole_BOSS) {
        session->ChangeRole(role);
        return true;
    }

    uint8_t expectBossPlayerCount = 0;
    if (false == mBossPlayerCount.compare_exchange_strong(expectBossPlayerCount, 1)) {
        return false;
    }

    session->ChangeRole(role);
    return true;
}

bool GameRoom::ReadyPlayer(SessionIdType id) {
    mSessionLock.ReadLock();
    if (not mSessionsInRoom.contains(id)) {
        return false;
    }
    mSessionLock.ReadUnlock();

    auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(id));
    if (nullptr == session) {
        return false;
    }
    
    ++mReadyPlayerCount;
    return session->Ready();
}

bool GameRoom::CancelPlayerReady(SessionIdType id) {
    mSessionLock.ReadLock();
    if (not mSessionsInRoom.contains(id)) {
        return false;
    }
    mSessionLock.ReadUnlock();

    auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(id));
    if (nullptr == session) {
        return false;
    }

    --mReadyPlayerCount;
    return session->CancelReady();
}

bool GameRoom::CheckAndStartGame() {
    if (not IsEveryPlayerReady()) {
        return false;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Register Start Game!!!");

    auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
    gServerFrame->AddTimerEvent(mRoomIdx, INVALID_SESSION_ID, excutionTime, TimerEventType::SCENE_TRANSITION_COUNTDOWN);
    mSceneTransitionCounter = SysClock::now();
    return true;
}

void GameRoom::BroadCastInGameRoom(SessionIdType sender, OverlappedSend* packet) {
    decltype(auto) sessionsInGameRoom = GetSessions();
    std::vector<std::shared_ptr<GameSession>> sessionList{ };
    decltype(auto) sessionLock = gServerCore->GetSessionManager()->GetSessionLock();
    {
        Lock::SRWLockGuard sessionGuard{ Lock::SRWLockMode::SRW_SHARED, sessionLock };
        for (auto& sessionId : sessionsInGameRoom) {
            if (sender == sessionId) {
                continue;
            }

            auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
            if (nullptr == session or SESSION_INLOBBY != session->GetSessionState()) {
                continue;
            }

            sessionList.push_back(session);
        }
    }

    for (auto& session : sessionList) {
        auto clonedPacket = FbsPacketFactory::ClonePacket(packet);
        session->RegisterSend(clonedPacket);
    }

    FbsPacketFactory::ReleasePacketBuf(packet);
}

void GameRoom::BroadCastInGameRoom(OverlappedSend* packet) {
    decltype(auto) sessionsInGameRoom = GetSessions();
    std::vector<std::shared_ptr<GameSession>> sessionList{ };
    decltype(auto) sessionLock = gServerCore->GetSessionManager()->GetSessionLock();
    {
        Lock::SRWLockGuard sessionGuard{ Lock::SRWLockMode::SRW_SHARED, sessionLock };
        for (auto& sessionId : sessionsInGameRoom) {
            auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
            if (nullptr == session or SESSION_INLOBBY != session->GetSessionState()) {
                continue;
            }

            sessionList.push_back(session);
        }
    }

    for (auto& session : sessionList) {
        auto clonedPacket = FbsPacketFactory::ClonePacket(packet);
        session->RegisterSend(clonedPacket);
    }

    FbsPacketFactory::ReleasePacketBuf(packet);
}

void GameRoom::OnSceneCountdownTick() {
    if (not IsEveryPlayerReady()) {
        mSceneTransitionCounter = SysClock::now();
        return;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "OnSceneCountdownTick");

    auto sceneTransitionTime = std::chrono::duration_cast<std::chrono::milliseconds>(SysClock::now() - mSceneTransitionCounter).count();
    if (SCENE_TRANSITION_COUNT <= sceneTransitionTime / 1000.0f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Start Game!!!");
        mStage.StartStage();

        auto packetSceneTransition = FbsPacketFactory::ChangeToNextSceneSC();
        BroadCastInGameRoom(packetSceneTransition);
        return;
    }

    auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
    gServerFrame->AddTimerEvent(mRoomIdx, INVALID_SESSION_ID, excutionTime, TimerEventType::SCENE_TRANSITION_COUNTDOWN);
}

GameRoomManager::GameRoomManager() {
    for (uint16_t roomIdx{ 0 }; auto& room : mGameRooms) {
        room = std::make_unique<GameRoom>(roomIdx);
        ++roomIdx;
    }
}

GameRoomManager::~GameRoomManager() { }

uint8_t GameRoomManager::GetLastErrorCode() {
    return LAST_ERROR_CODE;
}

std::unique_ptr<GameRoom>& GameRoomManager::GetRoom(uint16_t roomIdx) {
    return mGameRooms.at(roomIdx);
}

void GameRoomManager::InitGameRooms() {
    for (auto& room : mGameRooms) {
        room->GetStage().InitObjectManager("../Resources/Binarys/Collider/env1.bin");
    }
}

uint16_t GameRoomManager::TryInsertGameRoom(SessionIdType sessionId) {
    for (uint16_t roomIdx{ 0 }; auto& room : mGameRooms) {
        if (room->IsMaxSession()) {
            ++roomIdx;
            continue;
        }

        auto errorCode = room->TryInsertInRoom(sessionId);
        if (GameRoomError::SUCCESS_INSERT_SESSION_IN_ROOM == errorCode) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session[{}] Insert In GameRoom[{}]!", sessionId, roomIdx);
            LAST_ERROR_CODE = errorCode;
            return roomIdx;
        }

        ++roomIdx;
    }

    LAST_ERROR_CODE = GameRoomError::ERROR_ALL_ROOM_IS_FULL;
    return INSERT_GAME_ROOM_ERROR;
}

uint8_t GameRoomManager::TryRemoveGameRoom(uint16_t roomIdx, SessionIdType sessionId, bool lastReadyState, uint8_t lastSlotIndex) {
    auto& room = mGameRooms[roomIdx];
    auto errorCode = room->RemovePlayer(sessionId, lastReadyState, lastSlotIndex);
    if (GameRoomError::SUCCESS_REMOVE_SESSION_IN_ROOM != errorCode) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In TryRemoveGameRoom - ErrorCode: {}", errorCode);
        return errorCode;
    }

    auto packetExit = FbsPacketFactory::PlayerExitSC(sessionId);
    room->BroadCastInGameRoom(sessionId, packetExit);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session[{}] Remove From GameRoom[{}]!", sessionId, roomIdx);

    return errorCode;
}

Lock::SRWLock& GameRoomManager::GetSessionLock(uint16_t roomIdx) {
    return mGameRooms.at(roomIdx)->GetSessionLock();
}

SessionListInRoom& GameRoomManager::GetSessionsInRoom(uint16_t roomIdx) {
    return mGameRooms.at(roomIdx)->GetSessions();
}

bool GameRoomManager::ChangeRolePlayer(uint16_t roomIdx, SessionIdType id, Packets::PlayerRole role) {
    return mGameRooms.at(roomIdx)->ChangeRolePlayer(id, role);
}

bool GameRoomManager::ReadyPlayer(uint16_t roomIdx, SessionIdType id) {
    return mGameRooms.at(roomIdx)->ReadyPlayer(id);
}

bool GameRoomManager::CancelPlayerReady(uint16_t roomIdx, SessionIdType id) {
    return mGameRooms.at(roomIdx)->CancelPlayerReady(id);
}
