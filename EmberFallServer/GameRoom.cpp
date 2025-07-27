#include "pch.h"
#include "GameRoom.h"
#include "ServerFrame.h"
#include "HumanPlayerScript.h"

GameRoom::GameRoom(uint16_t roomIdx)
    : mRoomIdx{ roomIdx }, mGameRoomState{ GameRoomState::GAME_ROOM_STATE_LOBBY }, mStage{ Packets::GameStage_LOBBY, roomIdx } {
    for (uint8_t i = 0; i < MAX_PLAYER_IN_GAME_ROOM; ++i) {
        mSessionSlotIndices.push(i);
    }
}

GameRoom::~GameRoom() {}

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

    {
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
            return 255;
        }

        auto session = gServerFrame->GetSession(sessionId);
        if (nullptr == session) {
            return GameRoomError::ERROR_SESSION_EXISTS_IN_THIS_ROOM;
        }

        ++mPlayerCount;
        session->SetSlotIndex(slotIndex);
        session->SetName(std::format("PID {}", slotIndex));
        mSessionsInRoom.insert(sessionId);
    }

    bool expected = false;
    if (true == mHeartBeat.compare_exchange_strong(expected, true)) {
        gServerFrame->AddTimerEvent(INVALID_OBJ_ID, CHECK_SESSION_HEART_BEAT_DELAY, IoType::CHECK_SESSION_HEART_BEAT, mRoomIdx);
    }

    return GameRoomError::SUCCESS_INSERT_SESSION_IN_ROOM;
}

uint8_t GameRoom::RemovePlayer(SessionIdType id, Packets::PlayerRole lastRole, bool lastReadyState, uint8_t lastSlotIndex) {
    {
        Lock::SRWLockGuard sesseionGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionLock };
        if (not mSessionsInRoom.contains(id)) {
            return GameRoomError::ERROR_SESSION_NOT_EXISTS_IN_THIS_ROOM;
        }
        mSessionsInRoom.erase(id);
        mSessionSlotIndices.push(lastSlotIndex);
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

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session [{}] erased in GameRoom [{}], last slot: [{}]", id, mRoomIdx, lastSlotIndex);

    auto packetExit = FbsPacketFactory::PlayerExitSC(id);
    BroadCast(packetExit);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "BroadCast End!");

    return GameRoomError::SUCCESS_REMOVE_SESSION_IN_ROOM;
}

bool GameRoom::ChangeRolePlayer(SessionIdType id, Packets::PlayerRole role) {
    mSessionLock.ReadLock();
    if (not mSessionsInRoom.contains(id)) {
        mSessionLock.ReadUnlock();
        return false;
    }
    mSessionLock.ReadUnlock();

    auto session = gServerFrame->GetSession(id);
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
        mSessionLock.ReadUnlock();
        return false;
    }
    mSessionLock.ReadUnlock();

    auto session = gServerFrame->GetSession(id);
    if (nullptr == session) {
        return false;
    }

    ++mReadyPlayerCount;
    return session->Ready();
}

bool GameRoom::CancelPlayerReady(SessionIdType id) {
    mSessionLock.ReadLock();
    if (not mSessionsInRoom.contains(id)) {
        mSessionLock.ReadUnlock();
        return false;
    }
    mSessionLock.ReadUnlock();

    if (GameRoomState::GAME_ROOM_STATE_TRANSITION == mGameRoomState) {
        mTransitionInterruptFlag = true;
    }

    auto session = gServerFrame->GetSession(id);
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

    gServerFrame->AddTimerEvent(INVALID_SESSION_ID, SCENE_TRANSITION_EVENT_DELAY, IoType::SCENE_TRANSITION_COUNTDOWN, mRoomIdx);
    mSceneTransitionCounter = SysClock::now();

    mStageTransitionTarget = Packets::GameStage_LOBBY;

    auto packetStartTransition = FbsPacketFactory::StartSceneTransition(SCENE_TRANSITION_COUNT);
    BroadCast(packetStartTransition);
}

bool GameRoom::CheckAndStartGame() {
#ifdef DEV_MODE
    if (not IsEveryPlayerReady()) {
        return false;
    }
#else
    const uint8_t expectedBossCnt = 1;
    const uint8_t minimumPlayerCount = 2;
    if (not (IsEveryPlayerReady() and minimumPlayerCount <= mPlayerCount and expectedBossCnt == mBossPlayerCount)) {
        return false;
    }
#endif

    mGameRoomState = GameRoomState::GAME_ROOM_STATE_TRANSITION;

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Register Start Game!!!");

    auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
    gServerFrame->AddTimerEvent(INVALID_SESSION_ID, SCENE_TRANSITION_EVENT_DELAY, IoType::SCENE_TRANSITION_COUNTDOWN, mRoomIdx);
    mSceneTransitionCounter = SysClock::now();

    mStageTransitionTarget = Packets::GameStage_TERRAIN;

    auto packetStartTransition = FbsPacketFactory::StartSceneTransition(SCENE_TRANSITION_COUNT);
    BroadCast(packetStartTransition);

    return true;
}

void GameRoom::SpawnItem() {
    if (GameRoomState::GAME_ROOM_STATE_INGAME != mGameRoomState) {
        return;
    }

    auto stage = mStage.GetStageIdx();
    if (Packets::GameStage_LOBBY == stage or Packets::GameStage_LAST == stage) {
        return;
    }

    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    for (auto& sessionId : sessionsInGameRoom) {
        auto session = gServerFrame->GetSession(sessionId);
        if (nullptr == session) {
            continue;
        }

        auto userObj = session->GetUserObject();
        if (nullptr == userObj or nullptr == userObj->GetScript<HumanPlayerScript>()) {
            continue;
        }

        auto pos = userObj->GetPosition();

        auto potion = mStage.GetObjectManager()->SpawnObject(Packets::EntityType_ITEM_POTION);
        potion->GetTransform()->SetPosition(Random::GetRandVecInArea(GameProtocol::Logic::ITEM_SPAWN_AREA, pos));
        potion->GetTransform()->Update();
        potion->GetBoundingObject()->Update(potion->GetWorld());

        mStage.GetTerrainCollider().HandleTerrainCollision(potion);

        auto itemPos = potion->GetPosition();
        mStage.AddInSector(potion->GetId(), itemPos);
    }

    gServerFrame->AddTimerEvent(SYSTEM_ID, GameProtocol::Logic::ITEM_SPAWN_DELAY, IoType::SPAWN_ITEM, mRoomIdx);
}

void GameRoom::CheckSessionsHeartBeat() {
    decltype(auto) sessionList = GetSessions();
    mSessionLock.ReadLock();
    if (sessionList.empty()) {
        mSessionLock.ReadUnlock();

        bool expected = true;
        mHeartBeat.compare_exchange_strong(expected, false);
        return;
    }
    std::vector<SessionIdType> sessionsInGameRoom{ sessionList.begin(), sessionList.end() };
    mSessionLock.ReadUnlock();

    gServerFrame->CheckSessionsHeartBeat(sessionsInGameRoom);
    gGameRoomManager->GetRoom(0);
    gServerFrame->AddTimerEvent(INVALID_OBJ_ID, CHECK_SESSION_HEART_BEAT_DELAY, IoType::CHECK_SESSION_HEART_BEAT, mRoomIdx);
}

void GameRoom::CheckGameEnd() {
    if (GameRoomState::GAME_ROOM_STATE_INGAME != mGameRoomState) {
        return;
    }

    auto [isEnd, winner] = mIngameCondition.CheckGameEnd(mStage.GetStageIdx());
    if (not isEnd) {
        if (GameRoomState::GAME_ROOM_STATE_INGAME == mGameRoomState) {
            auto delay = GameProtocol::Logic::GAME_ROOM_CHECK_GAME_END_DELAY;
            gServerFrame->AddTimerEvent(INVALID_OBJ_ID, delay, IoType::CHECK_GAME_CONDITION, mRoomIdx);
        }
        return;
    }

    if (Packets::PlayerRole_BOSS == winner) {
        auto packetGameEnd = FbsPacketFactory::GameEndSC(winner);
        BroadCast(packetGameEnd);

        EndGameLoop();
        return;
    }

    mSessionLock.ReadLock();
    decltype(auto) sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();
    if (0 == sessionsInGameRoom.size()) {
        auto packetGameEnd = FbsPacketFactory::GameEndSC(winner);
        BroadCast(packetGameEnd);

        EndGameLoop();
        return;
    }

    if (Packets::GameStage_LAST != mStage.GetStageIdx()) {
        mGameRoomState = GameRoomState::GAME_ROOM_STATE_TRANSITION;

        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Register Start Game!!!");

        mStageTransitionTarget = static_cast<Packets::GameStage>(mStageTransitionTarget + 1);

        gServerFrame->AddTimerEvent(INVALID_SESSION_ID, SCENE_TRANSITION_EVENT_DELAY, IoType::SCENE_TRANSITION_COUNTDOWN, mRoomIdx);
        mSceneTransitionCounter = SysClock::now();

        auto packetStartTransition = FbsPacketFactory::StartSceneTransition(SCENE_TRANSITION_COUNT);
        BroadCast(packetStartTransition);
    }
    else {
        auto packetGameEnd = FbsPacketFactory::GameEndSC(winner);
        BroadCast(packetGameEnd);

        EndGameLoop();
    }
}

void GameRoom::ChangeToLobby() {
    mStage.EndStage();
    mGameRoomState = GameRoomState::GAME_ROOM_STATE_LOBBY;

    mSessionLock.ReadLock();
    decltype(auto) sessionsInGameRoom = GetSessions();
    for (auto& sessionId : sessionsInGameRoom) {
        auto session = gServerFrame->GetSession(sessionId);
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
}

void GameRoom::ChangeToStage1() {
    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    mReadyPlayerCount = 0;
    for (auto& sessionId : sessionsInGameRoom) {
        auto session = gServerFrame->GetSession(sessionId);
        if (nullptr == session) {
            continue;
        }

        session->CancelReady();
    }

    mGameRoomState = GameRoomState::GAME_ROOM_STATE_INGAME;

    auto delay = GameProtocol::Logic::GAME_ROOM_CHECK_GAME_END_DELAY;
    gServerFrame->AddTimerEvent(INVALID_OBJ_ID, delay, IoType::CHECK_GAME_CONDITION, mRoomIdx);

    auto humanCnt = mPlayerCount - mBossPlayerCount;
    auto gemCnt = humanCnt * 2;
    mStage.StartStage(gemCnt);
    mIngameCondition.InitGameCondition(humanCnt, mBossPlayerCount, gemCnt);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameRoom [{}]: Start Game!!!", mRoomIdx);

    auto packetSceneTransition = FbsPacketFactory::ChangeSceneSC(mStageTransitionTarget);
    BroadCast(packetSceneTransition);
    return;
}

void GameRoom::ChangeToNextStage() {
    mStage.EndStage();
    mGameRoomState = GameRoomState::GAME_ROOM_STATE_INGAME;
    mStage.StartStage(0, mStageTransitionTarget);

    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    mReadyPlayerCount = 0;
    for (auto& sessionId : sessionsInGameRoom) {
        auto session = gServerFrame->GetSession(sessionId);
        if (nullptr == session) {
            continue;
        }

        session->ChangeStage();
    }

    auto delay = GameProtocol::Logic::GAME_ROOM_CHECK_GAME_END_DELAY;
    gServerFrame->AddTimerEvent(INVALID_OBJ_ID, delay, IoType::CHECK_GAME_CONDITION, mRoomIdx);

    auto humanCnt = mPlayerCount - mBossPlayerCount;
    mIngameCondition.InitGameCondition(humanCnt, mBossPlayerCount, 0);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameRoom [{}]: Start Game!!!, Stage: {}", mRoomIdx, Packets::EnumNameGameStage(mStageTransitionTarget));

    auto packetSceneTransition = FbsPacketFactory::ChangeSceneSC(mStageTransitionTarget);
    BroadCast(packetSceneTransition);
    return;
}

void GameRoom::DebugChangeToNextStage() {
    auto winner = Packets::PlayerRole_HUMAN;

    if (Packets::GameStage_LAST != mStage.GetStageIdx()) {
        mGameRoomState = GameRoomState::GAME_ROOM_STATE_TRANSITION;

        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Register Start Game!!!");

        auto excutionTime = SysClock::now() + SCENE_TRANSITION_EVENT_DELAY;
        gServerFrame->AddTimerEvent(INVALID_SESSION_ID, SCENE_TRANSITION_EVENT_DELAY, IoType::SCENE_TRANSITION_COUNTDOWN, mRoomIdx);
        mSceneTransitionCounter = SysClock::now();

        mStageTransitionTarget = static_cast<Packets::GameStage>(mStageTransitionTarget + 1);

        auto packetStartTransition = FbsPacketFactory::StartSceneTransition(SCENE_TRANSITION_COUNT);
        BroadCast(packetStartTransition);
    }
    else {
        auto packetGameEnd = FbsPacketFactory::GameEndSC(winner);
        BroadCast(packetGameEnd);

        EndGameLoop();
    }
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
        if (0 == mIngameCondition.GetGemCount()) {
            decltype(auto) sessionsRef = GetSessions();
            mSessionLock.ReadLock();
            std::vector<SessionIdType> sessions{ sessionsRef.begin(), sessionsRef.end() };
            mSessionLock.ReadUnlock();

            mStage.NotifyAllOfGemDestroyed(sessions);
        }
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Remove Corrupted Gem, Gem Count : {}", mIngameCondition.GetGemCount());
    }
    break;

    case ObjectTag::BOSSPLAYER:
    {
        mIngameCondition.FetchSubBossCount();
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Remove Boss Player", mIngameCondition.GetBossCount());
    }
    break;

    case ObjectTag::PLAYER:
    {
        mIngameCondition.FetchSubHumanCount();
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Remove Human Player", mIngameCondition.GetAliveHumanCount());
    }
    break;

    default:
        break;
    }
}

void GameRoom::BroadCast(SessionIdType sender, OverlappedSend* packet) {
    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    for (auto& sessionId : sessionsInGameRoom) {
        if (sender == sessionId) {
            continue;
        }

        auto session = gServerFrame->GetSession(sessionId);
        if (nullptr == session or not session->ReadyToRecv()) {
            continue;
        }

        auto clonedPacket = FbsPacketFactory::ClonePacket(packet);
        if (false == session->RegisterSend(clonedPacket)) {
            gServerFrame->CloseSession(sessionId);
        }
    }

    FbsPacketFactory::ReleasePacketBuf(packet);
}

void GameRoom::BroadCast(OverlappedSend* packet) {
    mSessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = GetSessions();
    mSessionLock.ReadUnlock();

    for (auto& sessionId : sessionsInGameRoom) {
        auto session = gServerFrame->GetSession(sessionId);
        if (nullptr == session or not session->ReadyToRecv()) {
            continue;
        }

        auto clonedPacket = FbsPacketFactory::ClonePacket(packet);
        if (false == session->RegisterSend(clonedPacket)) {
            gServerFrame->CloseSession(sessionId);
        }
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
    if (Packets::GameStage_LOBBY == mStageTransitionTarget) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "OnSceneCountdownTick - Remain Time: {}s", std::max(0.0f, SCENE_TRANSITION_COUNT - sceneTransitionTime));
        if (SCENE_TRANSITION_COUNT_TO_LOBBY > sceneTransitionTime) {
            gServerFrame->AddTimerEvent(INVALID_SESSION_ID, SCENE_TRANSITION_EVENT_DELAY, IoType::SCENE_TRANSITION_COUNTDOWN, mRoomIdx);
            return;
        }
    }
    else {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "OnSceneCountdownTick - Remain Time: {}s", std::max(0.0f, SCENE_TRANSITION_COUNT - sceneTransitionTime));
        if (SCENE_TRANSITION_COUNT > sceneTransitionTime) {
            gServerFrame->AddTimerEvent(INVALID_SESSION_ID, SCENE_TRANSITION_EVENT_DELAY, IoType::SCENE_TRANSITION_COUNTDOWN, mRoomIdx);
            return;
        }
    }

    switch (mStageTransitionTarget) {
    case Packets::GameStage_LOBBY:
        ChangeToLobby();
        break;

    case Packets::GameStage_TERRAIN:
        ChangeToStage1();
        break;

    default:
        ChangeToNextStage();
        break;
    }
}

GameRoomManager::GameRoomManager() {}

GameRoomManager::~GameRoomManager() {}

uint8_t GameRoomManager::GetLastErrorCode() {
    return LAST_ERROR_CODE;
}

std::unique_ptr<GameRoom>& GameRoomManager::GetRoom(uint16_t roomIdx) {
    return mGameRooms.at(roomIdx);
}

void GameRoomManager::InitGameRooms() {
    for (uint16_t roomIdx{ 0 }; auto& room : mGameRooms) {
        room = std::make_unique<GameRoom>(roomIdx);
        ++roomIdx;
    }

    for (auto& room : mGameRooms) {
        room->GetStage().InitObjectManager();
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

GameCondition::GameCondition() {}

GameCondition::~GameCondition() {}

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

std::pair<bool, Packets::PlayerRole> GameCondition::CheckGameEnd(Packets::GameStage stage) {
#ifdef DEV_MODE
    if (Packets::GameStage_TERRAIN == stage) {
        if (0 == mGemCount) {
            return std::make_pair(true, Packets::PlayerRole_HUMAN);
        }
    }

    if ((0 == mAliveHumanCount + mBossCount)) {
        return std::make_pair(true, Packets::PlayerRole_HUMAN);
    }

#else
    if (Packets::GameStage_TERRAIN == stage) {
        if (0 == mGemCount) {
            return std::make_pair(true, Packets::PlayerRole_HUMAN);
        }
    }

    if (0 == mBossCount) {
        return std::make_pair(true, Packets::PlayerRole_HUMAN);
    }

    if (0 == mAliveHumanCount) {
        return std::make_pair(true, Packets::PlayerRole_BOSS);
    }
#endif

    return std::make_pair(false, Packets::PlayerRole_HUMAN);
}
