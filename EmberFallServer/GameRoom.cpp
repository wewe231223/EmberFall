#include "pch.h"
#include "GameRoom.h"

GameRoom::GameRoom(uint16_t roomIdx)
    : mRoomIdx{ roomIdx }, mGameRoomState{ GAME_ROOM_STATE_LOBBY }, mStage{ GameStage::STAGE1, roomIdx } { }

GameRoom::~GameRoom() { }

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

    mSessionsInRoom.insert(sessionId);

    return GameRoomError::SUCCESS_INSERT_SESSION_IN_ROOM;
}

uint8_t GameRoom::RemovePlayer(SessionIdType id) {
    Lock::SRWLockGuard sessionGaurd{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionLock };
    if (not mSessionsInRoom.contains(id)) {
        return GameRoomError::ERROR_SESSION_NOT_EXISTS_IN_THIS_ROOM;
    }

    mSessionsInRoom.erase(id);
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session [{}] erased in GameRoom [{}]", id, mRoomIdx);

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

bool GameRoom::CheckAndStartGame() {
    // TODO - Check if all players are in the READY state and send ChangeToNextSceneSC packet to clients
    return false;
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

uint8_t GameRoomManager::TryRemoveGameRoom(uint16_t roomIdx, SessionIdType sessionId) {
    auto& room = mGameRooms[roomIdx];
    auto errorCode = room->RemovePlayer(sessionId);
    if (GameRoomError::SUCCESS_REMOVE_SESSION_IN_ROOM != errorCode) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In TryRemoveGameRoom - ErrorCode: {}", errorCode);
        return errorCode;
    }

    std::vector<std::shared_ptr<Session>> otherSessions{ };
    decltype(auto) sessionLock = gServerCore->GetSessionManager()->GetSessionLock();
    {
        Lock::SRWLockGuard sessionGuard{ Lock::SRWLockMode::SRW_SHARED, sessionLock };
        for (auto& otherSessionId : GetRoom(roomIdx)->GetSessions()) {
            if (sessionId == otherSessionId) {
                continue;
            }

            auto otherSession = gServerCore->GetSessionManager()->GetSession(otherSessionId);
            if (nullptr == otherSession or false == otherSession->IsConnected()) {
                continue;
            }

            otherSessions.push_back(otherSession);
        }
    }

    auto packetExit = FbsPacketFactory::PlayerExitSC(sessionId);
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "ExitPacket Send Count: {}", otherSessions.size());
    for (auto& otherSession : otherSessions) {
        auto clonedPacket = FbsPacketFactory::ClonePacket(packetExit);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "To Session[{}] Send ExitPacket!", otherSession->GetId());
        otherSession->RegisterSend(clonedPacket);
    }

    FbsPacketFactory::ReleasePacketBuf(packetExit);

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