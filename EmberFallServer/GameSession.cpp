#include "pch.h"
#include "GameSession.h"
#include "FbsPacketProcessFn.h"
#include "PlayerScript.h"
#include "ObjectManager.h"
#include "Input.h"
#include "Resources.h"
#include "GameRoom.h"

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

    if (nullptr != mUserObject) {
        gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->RemoveInSector(GetId(), mUserObject->GetPosition());
        mUserObject->Reset();
    }

    // TODO - Remove In GameRoom
    auto result = gGameRoomManager->TryRemoveGameRoom(myRoom, myId, mPlayerRole, mReady, mSlotIndexInLobby);
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameSession Destructor: Session Erase From Room Error: {}", result);

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
    EnterLobby();
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
        return;
    }

    decltype(auto) dataPtr = reinterpret_cast<const uint8_t* const>(mOverlappedRecv.buffer.data());
    ProcessPackets(sharedThis, dataPtr, dataSize);
    if (false == IsConnected()) {
        return;
    }

    if (0 < mPrevRemainSize) {
        std::copy(remainBegin, dataEnd, dataBeg);
    }

    RegisterRecv();
}

void GameSession::InitUserObject() {
    static auto TestPos = SimpleMath::Vector3::Zero;
    const static auto PosInc = SimpleMath::Vector3::Left * 2.0f;

    auto myRoom = GetMyRoomIdx();

    mUserObject = gGameRoomManager->GetRoom(myRoom)->GetStage().GetObjectManager()->GetObjectFromId(GetId());
    mUserObject->mSpec.active = true;
    mUserObject->CreateScript<PlayerScript>(mUserObject, std::make_shared<Input>());
    mUserObject->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
    mUserObject->GetTransform()->SetY(0.0f);

    mUserObject->mSpec.hp = 100.0f;
    mUserObject->mSpec.damage = 10.0f;

    auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
    auto player = mUserObject->GetScript<PlayerScript>();
    if (nullptr == player) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "In InitUser Object -> PlayerScript is Null");
    }

    player->SetOwnerSession(sharedFromThis);

    player->GetTransform()->Translate(TestPos);
    TestPos += PosInc;
    mUserObject->Init();

    const ObjectSpec spec = mUserObject->mSpec;
    const auto yaw = mUserObject->GetEulerRotation().y;
    const auto pos = mUserObject->GetPosition();
    const auto anim = mUserObject->mAnimationStateMachine.GetCurrState();
    decltype(auto) packetAppeared = FbsPacketFactory::ObjectAppearedSC(GetId(), spec.entity, yaw, anim, spec.hp, pos);

    RegisterSend(packetAppeared);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Send Appeared My Player: {}", GetId());

    const auto range = mUserObject->GetScript<PlayerScript>()->GetViewList().mViewRange.Count();

    gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->AddInSector(GetId(), pos);
    gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->UpdatePlayerViewList(mUserObject, pos, range);
}

void GameSession::EnterLobby() {
    mSessionState = SESSION_INLOBBY;
}

void GameSession::EnterInGame() {
    mSessionState = SESSION_INGAME;
    InitUserObject();
}

bool GameSession::Ready() {
    if (true == mReady) {
        return false;
    }

    mReady = true;
    return true;
}

bool GameSession::CancelReady() {
    if (false == mReady) {
        return false;
    }

    mReady = false;
    return true;
}

void GameSession::ChangeRole(Packets::PlayerRole role) {
    mPlayerRole = role;
}

std::shared_ptr<GameObject> GameSession::GetUserObject() const {
    return mUserObject;
}

void GameSession::SetSlotIndex(uint8_t slotIndex) {
    mSlotIndexInLobby = slotIndex;
}

void GameSession::SetName(const std::string& str) {
    mName = str;
}

uint8_t GameSession::GetSessionState() const {
    return mSessionState.load();
}

uint8_t GameSession::GetSlotIndex() const {
    return mSlotIndexInLobby;
}

Packets::PlayerRole GameSession::GetPlayerRole() const {
    return mPlayerRole;
}

std::string GameSession::GetName() const {
    return mName;
}

std::string_view GameSession::GetNameView() const {
    return mName;
}

bool GameSession::GetReadyState() const {
    return mReady;
}
