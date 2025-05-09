#include "pch.h"
#include "GameRoom.h"
#include "ServerFrame.h"

GameRoom::GameRoom(uint16_t roomIdx)
    : mRoomIdx{ roomIdx }, mGameRoomState{ GameRoomState::GAME_ROOM_STATE_LOBBY }, mStage{ GameStage::STAGE1, roomIdx } { 
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
    if (GameRoomState::GAME_ROOM_STATE_LOBBY != mGameRoomState) {
        return GameRoomError::ERROR_ROOM_STATE_IS_INGAME;
    }

    Lock::SRWLockGuard sessionGaurd{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionLock };
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

    bool expected = false;
    if (true == mHeartBeat.compare_exchange_strong(expected, true)) {
        auto executionTime = SysClock::now() + CHECK_SESSION_HEART_BEAT_DELAY;
        gServerFrame->AddTimerEvent(mRoomIdx, INVALID_OBJ_ID, executionTime, TimerEventType::CHECK_SESSION_HEART_BEAT);
    }

    return GameRoomError::SUCCESS_INSERT_SESSION_IN_ROOM;
}

uint8_t GameRoom::RemovePlayer(SessionIdType id, Packets::PlayerRole lastRole, bool lastReadyState, uint8_t lastSlotIndex) {
    {
        Lock::SRWLockGuard sessionGaurd{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionLock };
        if (not mSessionsInRoom.contains(id)) {
            return GameRoomError::ERROR_SESSION_NOT_EXISTS_IN_THIS_ROOM;
        }

        if (mGameRoomState == GameRoomState::GAME_ROOM_STATE_TRANSITION) {
            mTransitionInterruptFlag = true;
        }

        if (Packets::PlayerRole_BOSS == lastRole) {
            uint8_t expectedBossCount = 1;
            mBossPlayerCount.compare_exchange_strong(expectedBossCount, 0);
        }

        if (true == lastReadyState) {
            --mReadyPlayerCount;
        }

        --mPlayerCount;

        if (GameRoomState::GAME_ROOM_STATE_LOBBY != mGameRoomState and 0 == mPlayerCount) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Boss Player Count zero or Player Count is zero");
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Now GameRoom [{}] state is GAME_ROOM_STATE_LOBBBY", mRoomIdx);

            mStage.EndStage();
            mGameRoomState = GameRoomState::GAME_ROOM_STATE_LOBBY;
        }

        mSessionsInRoom.erase(id);
        mSessionSlotIndices.push(lastSlotIndex);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session [{}] erased in GameRoom [{}], last slot: [{}]", id, mRoomIdx, lastSlotIndex);
    }

    auto packetExit = FbsPacketFactory::PlayerExitSC(id);
    BroadCast(packetExit);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "BroadCast End!"); 

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

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "allocated buffers: {}", SendBufferFactory::mSendBuffDebugger.load());
#endif
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

    if (GameRoomState::GAME_ROOM_STATE_TRANSITION == mGameRoomState) {
        mTransitionInterruptFlag = true;
    }

    auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(id));
    if (nullptr == session) {
        return false;
    }

    --mReadyPlayerCount;
    return session->CancelReady();
}

void GameRoom::EndGameLoop() {
    mGameRoomState = GameRoomState::GAME_ROOM_STATE_TRANSITION;

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "allocated buffers: {}", SendBufferFactory::mSendBuffDebugger.load());
#endif
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameRoom[{}]: Register End Game!!!", mRoomIdx);

    auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
    gServerFrame->AddTimerEvent(mRoomIdx, INVALID_SESSION_ID, excutionTime, TimerEventType::SCENE_TRANSITION_COUNTDOWN);
    mSceneTransitionCounter = SysClock::now();

    mStageTransitionTarget = Packets::GameStage_LOBBY;

    auto packetStartTransition = FbsPacketFactory::StartSceneTransition(SCENE_TRANSITION_COUNT);
    BroadCast(packetStartTransition);
}

bool GameRoom::CheckAndStartGame() {
    if (not IsEveryPlayerReady() or 0 == mPlayerCount) {
        return false;
    }

    mGameRoomState = GameRoomState::GAME_ROOM_STATE_TRANSITION;

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Register Start Game!!!");

    auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
    gServerFrame->AddTimerEvent(mRoomIdx, INVALID_SESSION_ID, excutionTime, TimerEventType::SCENE_TRANSITION_COUNTDOWN);
    mSceneTransitionCounter = SysClock::now();

    mStageTransitionTarget = Packets::GameStage_TERRAIN;

    auto packetStartTransition = FbsPacketFactory::StartSceneTransition(SCENE_TRANSITION_COUNT);
    BroadCast(packetStartTransition);

    return true;
}

void GameRoom::CheckSessionsHeartBeat() { 
    decltype(auto) sessionList = GetSessions();
    mSessionLock.ReadLock();
    if (sessionList.empty()) {
        mSessionLock.ReadUnlock();

        bool expected = true;
        mHeartBeat.compare_exchange_strong(expected, false);
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GameRoom [{}]: Check Failure HeartBeat", mRoomIdx);
        return;
    }
    std::vector<SessionIdType> sessionsInGameRoom{ sessionList.begin(), sessionList.end() };
    mSessionLock.ReadUnlock();

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GameRoom [{}]: Check HeartBeat", mRoomIdx);
    gServerCore->GetSessionManager()->CheckSessionsHeartBeat(sessionsInGameRoom);

    auto executionTime = SysClock::now() + CHECK_SESSION_HEART_BEAT_DELAY;
    gServerFrame->AddTimerEvent(mRoomIdx, INVALID_OBJ_ID, executionTime, TimerEventType::CHECK_SESSION_HEART_BEAT);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GameRoom [{}]: Check HeartBeat End", mRoomIdx);
}

void GameRoom::CheckGameEnd() {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GameRoom [{}]: Check Game End", mRoomIdx);
#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "allocated buffers: {}", SendBufferFactory::mSendBuffDebugger.load());
#endif
    auto [isEnd, winner] = mIngameCondition.CheckGameEnd();
    if (not isEnd) {
        if (GameRoomState::GAME_ROOM_STATE_INGAME == mGameRoomState) {
            auto executionTime = SysClock::now() + GameProtocol::Logic::GAME_ROOM_CHECK_GAME_END_DELAY;
            gServerFrame->AddTimerEvent(mRoomIdx, INVALID_OBJ_ID, executionTime, TimerEventType::CHECK_GAME_CONDITION);
        }
        return;
    }

    auto packetGameEnd = FbsPacketFactory::GameEndSC(winner);
    BroadCast(packetGameEnd);

    EndGameLoop();
}

void GameRoom::ChangeToLobby() {
    mStage.EndStage();
    mGameRoomState = GameRoomState::GAME_ROOM_STATE_LOBBY;

    mSessionLock.ReadLock();
    decltype(auto) sessionsInGameRoom = GetSessions();
    for (auto& sessionId : sessionsInGameRoom) {
        auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
        if (nullptr == session) {
            continue;
        }

        session->EnterLobby();
        session->CancelReady();
    }
    mSessionLock.ReadUnlock();

    mIngameCondition.Reset();

    auto packetToLobby = FbsPacketFactory::ChangeSceneSC(Packets::GameStage_LOBBY);
    BroadCast(packetToLobby);
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameRoom [{}]: Change To Lobby!!!", mRoomIdx);
#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "allocated buffers: {}", SendBufferFactory::mSendBuffDebugger.load());
#endif
}

void GameRoom::ChangeToStage1() {
    mSessionLock.ReadLock();
    mReadyPlayerCount = 0;
    decltype(auto) sessionsInGameRoom = GetSessions();
    for (auto& sessionId : sessionsInGameRoom) {
        auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
        if (nullptr == session) {
            continue;
        }

        session->CancelReady();
    }
    mSessionLock.ReadUnlock();

    mGameRoomState = GameRoomState::GAME_ROOM_STATE_INGAME;

    auto executionTime = SysClock::now() + GameProtocol::Logic::GAME_ROOM_CHECK_GAME_END_DELAY;
    gServerFrame->AddTimerEvent(mRoomIdx, INVALID_OBJ_ID, executionTime, TimerEventType::CHECK_GAME_CONDITION);
    mStage.StartStage();
    mIngameCondition.InitGameCondition(mPlayerCount - mBossPlayerCount, mBossPlayerCount, 1);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameRoom [{}]: Start Game!!!", mRoomIdx);

    auto packetSceneTransition = FbsPacketFactory::ChangeSceneSC(mStageTransitionTarget);
    BroadCast(packetSceneTransition);
    return;
}

void GameRoom::NotifyDestructedObject(ObjectTag tag) {
    if (GameRoomState::GAME_ROOM_STATE_INGAME != mGameRoomState) {
        return;
    }

    // Gem Count
    switch (tag) {
    case ObjectTag::CORRUPTED_GEM:
    {
        mIngameCondition.FetchSubGemCount();
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Remove Corrupted Gem, Gem Count : {}", mIngameCondition.GetGemCount());
        break;
    }

    case ObjectTag::PLAYER:
    {
        mIngameCondition.FetchSubHumanCount();
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Remove Human Player", mIngameCondition.GetAliveHumanCount());
        break;
    }

    case ObjectTag::BOSSPLAYER:
    {
        mIngameCondition.FetchSubBossCount();
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Remove Boss Player", mIngameCondition.GetBossCount());
        break;
    }

    default:
        break;
    }
}

void GameRoom::BroadCast(SessionIdType sender, OverlappedSend* packet) {
    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    std::vector<std::shared_ptr<GameSession>> sessionList{ };
    for (auto& sessionId : sessionsInGameRoom) {
        if (sender == sessionId) {
            continue;
        }

        auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
        if (nullptr == session) {
            continue;
        }

        sessionList.push_back(session);
    }
    
    for (auto& session : sessionList) {
        auto clonedPacket = FbsPacketFactory::ClonePacket(packet);
        session->RegisterSend(clonedPacket);
    }

    FbsPacketFactory::ReleasePacketBuf(packet);
}

void GameRoom::BroadCast(OverlappedSend* packet) {
    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    std::vector<std::shared_ptr<GameSession>> sessionList{ };
    for (auto& sessionId : sessionsInGameRoom) {
        auto session = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(sessionId));
        if (nullptr == session) {
            continue;
        }

        sessionList.push_back(session);
    }

    for (auto& session : sessionList) {
        auto clonedPacket = FbsPacketFactory::ClonePacket(packet);
        session->RegisterSend(clonedPacket);
    }

    FbsPacketFactory::ReleasePacketBuf(packet);
}

void GameRoom::OnSceneCountdownTick() {
    if (GameRoomState::GAME_ROOM_STATE_LOBBY == mGameRoomState and not IsEveryPlayerReady()) {
        mSceneTransitionCounter = SysClock::now();
        auto pakcetCancelTransition = FbsPacketFactory::CancelSceneTransition();
        BroadCast(pakcetCancelTransition);
        return;
    }

    if (true == mTransitionInterruptFlag and Packets::GameStage_LOBBY != mStageTransitionTarget) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GameRoom [{}]: Interrupt GameScene Transition - cancel transition", mRoomIdx);
        mTransitionInterruptFlag = false;
        CheckAndStartGame();

        mSceneTransitionCounter = SysClock::now();
        auto pakcetCancelTransition = FbsPacketFactory::CancelSceneTransition();
        BroadCast(pakcetCancelTransition);
        return;
    }

    auto sceneTransitionTime = std::chrono::duration_cast<std::chrono::milliseconds>(SysClock::now() - mSceneTransitionCounter).count() / 1000.0f;
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "OnSceneCountdownTick - Remain Time: {}s", std::max(0.0f, SCENE_TRANSITION_COUNT - sceneTransitionTime));
    if (SCENE_TRANSITION_COUNT > sceneTransitionTime) {
        auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
        gServerFrame->AddTimerEvent(mRoomIdx, INVALID_SESSION_ID, excutionTime, TimerEventType::SCENE_TRANSITION_COUNTDOWN);
        return;
    }

    switch (mStageTransitionTarget) {
    case Packets::GameStage_LOBBY:
        ChangeToLobby();
        break;

    case Packets::GameStage_TERRAIN:
        ChangeToStage1();
        break;
    }
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
        room->CheckSessionsHeartBeat();
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
    return GameRoomError::INSERT_GAME_ROOM_ERROR;
}

uint8_t GameRoomManager::TryRemoveGameRoom(uint16_t roomIdx, SessionIdType sessionId, Packets::PlayerRole lastRole, bool lastReadyState, uint8_t lastSlotIndex) {
    auto& room = mGameRooms[roomIdx];
    auto errorCode = room->RemovePlayer(sessionId, lastRole, lastReadyState, lastSlotIndex);
    if (GameRoomError::SUCCESS_REMOVE_SESSION_IN_ROOM != errorCode) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In TryRemoveGameRoom - ErrorCode: {}", errorCode);
        return errorCode;
    }

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

GameCondition::GameCondition() { }

GameCondition::~GameCondition() { }

uint8_t GameCondition::GetAliveHumanCount() const {
    return mAliveHumanCount;
}

uint8_t GameCondition::GetBossCount() const {
    return mBossCount;
}

uint8_t GameCondition::GetGemCount() const {
    return mGemCount;
}

void GameCondition::FetchSubHumanCount() {
    if (0 == mAliveHumanCount) {
        return;
    }

    mAliveHumanCount.fetch_sub(1);
}

void GameCondition::FetchSubGemCount() {
    if (0 == mGemCount) {
        return;
    }

    mGemCount.fetch_sub(1);
}

void GameCondition::FetchSubBossCount() {
    if (0 == mBossCount) {
        return;
    }

    mBossCount.fetch_sub(1);
}

void GameCondition::Reset() {
    mAliveHumanCount = 0;
    mGemCount = 0;
    mBossCount = 0;
}

void GameCondition::InitGameCondition(uint8_t humanCount, uint8_t bossCount, uint8_t gemCount) {
    mAliveHumanCount = humanCount;
    mBossCount = bossCount;
    mGemCount = gemCount;
}

std::pair<bool, Packets::PlayerRole> GameCondition::CheckGameEnd() {
    auto pair = std::make_pair(false, Packets::PlayerRole_HUMAN);
    if (0 == mGemCount) {
        pair.first = true;
        return pair;
    }

    //if (0 == mBossCount) {
    //    pair.first = true;
    //    return pair;
    //}

    if (0 == mAliveHumanCount) {
        pair.first = true;
        pair.second = Packets::PlayerRole_BOSS;
        return pair;
    }

    return pair;
}
