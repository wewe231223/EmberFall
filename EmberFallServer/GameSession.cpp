#include "pch.h"
#include "GameSession.h"
#include "ServerFrame.h"
#include "FbsPacketProcessFn.h"
#include "ObjectManager.h"
#include "Input.h"
#include "Resources.h"
#include "GameRoom.h"
#include "HumanPlayerScript.h"
#include "BossPlayerScript.h"

#include "Sector.h"

GameSession::GameSession() 
    : Session{ NetworkType::SERVER }, mSessionState{ SESSION_CONNECT } { }

GameSession::~GameSession() { 
    if (IsClosed()) {
        return;
    }

    Close();
}

void GameSession::Close() {
    auto myId = static_cast<SessionIdType>(GetId());
    auto myRoom = GetMyRoomIdx();

    auto executionTime = SysClock::now();
    SessionLobbyInfo info = mLobbyInfo;
    gServerFrame->AddTimerEvent(myRoom, myId, executionTime, TimerEventType::REMOVE_PLAYER_IN_ROOM, info);

    if (nullptr != mUserObject) {
        gServerFrame->AddTimerEvent(myRoom, myId, executionTime, TimerEventType::REMOVE_NPC, info);
        mUserObject = nullptr;
    }

    mSessionState = SESSION_CLOSE;

    Session::Close();
}

void GameSession::OnConnect() {
    Session::OnConnect();

    auto myId = static_cast<SessionIdType>(GetId());
    auto gameRoomIdx = gGameRoomManager->TryInsertGameRoom(myId);
    if (GameRoomError::INSERT_GAME_ROOM_ERROR == gameRoomIdx) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Session Insert Failure Error Core: [{}]", GameRoomManager::GetLastErrorCode());
        Session::Close();
        return;
    }
    
    SetRoomIdx(gameRoomIdx);
}

void GameSession::ProcessRecv(INT32 numOfBytes) {
    mOverlappedRecv.owner.reset();
    if (0 >= numOfBytes) {
        gServerCore->GetSessionManager()->CloseSession(static_cast<SessionIdType>(GetId()));
        return;
    }

    auto dataBeg = mOverlappedRecv.buffer.begin();
    auto dataEnd = dataBeg + numOfBytes;
    auto remainBegin = ValidatePackets(dataBeg, dataEnd);
    mPrevRemainSize = std::distance(remainBegin, dataEnd);
    auto dataSize = numOfBytes - mPrevRemainSize;
    
    // 패킷 처리
    auto sharedThis = std::static_pointer_cast<GameSession>(shared_from_this());
    if (nullptr == sharedThis) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Process Recv: Create Shared This Error In GameSession!!!");
        Crash("");
        return;
    }

    decltype(auto) dataPtr = reinterpret_cast<const uint8_t* const>(mOverlappedRecv.buffer.data());
    ProcessPackets(sharedThis, dataPtr, dataSize);
    if (false == IsConnected()) {
        gServerCore->GetSessionManager()->CloseSession(static_cast<SessionIdType>(GetId()));
        Crash("");
        return;
    }

    if (0 < mPrevRemainSize) {
        std::move(remainBegin, dataEnd, dataBeg);
    }

    RegisterRecv();
}

void GameSession::InitUserObject() {
    static auto TestPos = SimpleMath::Vector3::Zero;
    const static auto PosInc = SimpleMath::Vector3::Left * 2.0f;

    if (nullptr != mUserObject) {
        return;
    }

    auto myRoom = GetMyRoomIdx();
    auto myId = GetId();

    mUserObject = gGameRoomManager->GetRoom(myRoom)->GetStage().GetPlayer(GetId());

    InitPlayerScript();

    mUserObject->mSpec.active = true;
    mUserObject->mSpec.entity = static_cast<Packets::EntityType>(mLobbyInfo.lastRole);
    mUserObject->mSpec.hp = GameProtocol::Logic::MAX_HP;
    
    mUserObject->GetTransform()->Translate(TestPos);
    mUserObject->GetTransform()->SetY(0.0f);
    mUserObject->Init();
    mUserObject->mWeaponSystem.SetWeapon(mUserObject->mSpec.entity, mUserObject->mSpec.damage);

    const ObjectSpec spec = mUserObject->mSpec;
    const auto yaw = mUserObject->GetEulerRotation().y;
    const auto pos = mUserObject->GetPosition();
    const auto anim = mUserObject->mAnimationStateMachine.GetCurrState();

    decltype(auto) packetAppeared = FbsPacketFactory::ObjectAppearedSC(myId, spec.entity, yaw, anim, spec.hp, pos);
    RegisterSend(packetAppeared);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Send Appeared My Player: {}", myId);

    auto script = mUserObject->GetScript<PlayerScript>();
    if (nullptr == script) {
        return;
    }

    const float range = script->GetViewList().mViewRange.Count();

    gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->AddInSector(myId, pos);
    gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->UpdatePlayerViewList(mUserObject, pos, range);
}

void GameSession::InitPlayerScript() {
    switch (mLobbyInfo.lastRole) {
    case Packets::PlayerRole_HUMAN_ARCHER:
    {
        mUserObject->CreateScript<HumanPlayerScript>(mUserObject, std::make_shared<Input>());
        mUserObject->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        mUserObject->mAnimationStateMachine.Init(ANIM_KEY_ARCHER);
        mUserObject->GetTransform()->SetPosition(Random::GetRandVecInArea(GameProtocol::Logic::PLAYER_SPAWN_AREA, SimpleMath::Vector3{ 200.0f, 0.0f, 200.0f }));

        auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
        auto player = mUserObject->GetScript<HumanPlayerScript>();
        if (nullptr == player) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In InitUser Object -> PlayerScript is Null");
        }

        player->SetOwnerSession(sharedFromThis);
        break;
    }

    case Packets::PlayerRole_HUMAN_SWORD:
    {
        mUserObject->CreateScript<HumanPlayerScript>(mUserObject, std::make_shared<Input>());
        mUserObject->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        mUserObject->mAnimationStateMachine.Init(ANIM_KEY_SHIELD_MAN);
        mUserObject->GetTransform()->SetPosition(Random::GetRandVecInArea(GameProtocol::Logic::PLAYER_SPAWN_AREA, SimpleMath::Vector3{ 200.0f, 0.0f, 200.0f }));

        auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
        auto player = mUserObject->GetScript<HumanPlayerScript>();
        if (nullptr == player) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In InitUser Object -> PlayerScript is Null");
        }

        player->SetOwnerSession(sharedFromThis);
        break;
    }

    case Packets::PlayerRole_HUMAN_LONGSWORD:
    {
        mUserObject->CreateScript<HumanPlayerScript>(mUserObject, std::make_shared<Input>());
        mUserObject->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        mUserObject->mAnimationStateMachine.Init(ANIM_KEY_LONGSWORD_MAN);
        mUserObject->GetTransform()->SetPosition(Random::GetRandVecInArea(GameProtocol::Logic::PLAYER_SPAWN_AREA, SimpleMath::Vector3{ 200.0f, 0.0f, 200.0f }));

        auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
        auto player = mUserObject->GetScript<HumanPlayerScript>();
        if (nullptr == player) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In InitUser Object -> PlayerScript is Null");
        }

        player->SetOwnerSession(sharedFromThis);
        break;
    }

    case Packets::PlayerRole_HUMAN_MAGICIAN:
    {
        mUserObject->CreateScript<HumanPlayerScript>(mUserObject, std::make_shared<Input>());
        mUserObject->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        mUserObject->mAnimationStateMachine.Init(ANIM_KEY_MAGICIAN);
        mUserObject->GetTransform()->SetPosition(Random::GetRandVecInArea(GameProtocol::Logic::PLAYER_SPAWN_AREA, SimpleMath::Vector3{ 200.0f, 0.0f, 200.0f }));

        auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
        auto player = mUserObject->GetScript<HumanPlayerScript>();
        if (nullptr == player) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In InitUser Object -> PlayerScript is Null");
        }

        player->SetOwnerSession(sharedFromThis);
        break;
    }

    case Packets::PlayerRole_BOSS:
    {
        mUserObject->CreateScript<BossPlayerScript>(mUserObject, std::make_shared<Input>());
        mUserObject->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_DEMON).bb);
        mUserObject->mAnimationStateMachine.Init(ANIM_KEY_DEMON);
        mUserObject->GetTransform()->SetPosition(SimpleMath::Vector3{ -200.0f, 0.0f, -200.0f });

        auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
        auto player = mUserObject->GetScript<BossPlayerScript>();
        if (nullptr == player) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In InitUser Object -> PlayerScript is Null");
        }

        player->SetOwnerSession(sharedFromThis);
        break;
    }

    default:
        break;
    }
}

void GameSession::EnterLobby() {
    mSessionState = SESSION_INLOBBY;

    auto myId = static_cast<SessionIdType>(GetId());
    auto myRoom = GetMyRoomIdx();

    auto executionTime = SysClock::now();
    if (nullptr != mUserObject) {
        gServerFrame->AddTimerEvent(myRoom, myId, executionTime, TimerEventType::REMOVE_NPC);
        mUserObject = nullptr;
    }
}

void GameSession::EnterInGame() {
    mSessionState = SESSION_INGAME;
    InitUserObject();
}

bool GameSession::Ready() {
    if (true == mLobbyInfo.readyState) {
        return false;
    }

    mLobbyInfo.readyState = true;
    return true;
}

bool GameSession::CancelReady() {
    if (false == mLobbyInfo.readyState) {
        return false;
    }

    mLobbyInfo.readyState = false;
    return true;
}

void GameSession::ChangeRole(Packets::PlayerRole role) {
    mLobbyInfo.lastRole = role;
}

bool GameSession::ReadyToRecv() const {
    return mSessionState & SESSION_READY_TO_RECV;
}

std::shared_ptr<GameObject> GameSession::GetUserObject() const {
    return mUserObject;
}

void GameSession::SetSlotIndex(uint8_t slotIndex) {
    mLobbyInfo.sessionSlot = slotIndex;
}

void GameSession::SetName(const std::string& str) {
    mName = str;
}

uint8_t GameSession::GetSessionState() const {
    return mSessionState.load();
}

uint8_t GameSession::GetSlotIndex() const {
    return mLobbyInfo.sessionSlot;
}

Packets::PlayerRole GameSession::GetPlayerRole() const {
    return mLobbyInfo.lastRole;
}

std::string GameSession::GetName() const {
    return mName;
}

std::string_view GameSession::GetNameView() const {
    return mName;
}

bool GameSession::GetReadyState() const {
    return mLobbyInfo.readyState;
}
