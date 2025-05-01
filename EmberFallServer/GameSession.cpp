#include "pch.h"
#include "GameSession.h"
#include "FbsPacketProcessFn.h"
#include "PlayerScript.h"
#include "ObjectManager.h"
#include "Input.h"
#include "Resources.h"

#include "Sector.h"

GameSession::GameSession() 
    : Session{ NetworkType::SERVER }, mSessionState{ SESSION_CONNECT } { }

GameSession::~GameSession() { 
    gSectorSystem->RemoveInSector(GetId(), mUserObject->GetPosition());
    mUserObject->Reset();

    mSessionState = SESSION_CLOSE;

    Session::Close();
}

void GameSession::OnConnect() {
    Session::OnConnect();
}

void GameSession::ProcessRecv(INT32 numOfBytes) {
    mOverlappedRecv.owner.reset();
    if (0 >= numOfBytes) {
        Disconnect();
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

    if (0 < mPrevRemainSize) {
        std::copy(remainBegin, dataEnd, dataBeg);
    }

    RegisterRecv();
}

void GameSession::InitUserObject() {
    static auto TestPos = SimpleMath::Vector3::Zero;
    const static auto PosInc = SimpleMath::Vector3::Left * 2.0f;

    mUserObject = gObjectManager->GetObjectFromId(GetId());
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

    gSectorSystem->AddInSector(GetId(), pos);
    gSectorSystem->UpdatePlayerViewList(mUserObject, pos, range);
}

void GameSession::EnterLobby() {
    mSessionState = SESSION_INLOBBY;
}

void GameSession::EnterInGame() {
    mSessionState = SESSION_INGAME;
    InitUserObject();
}

void GameSession::Ready(Packets::PlayerRole role) {
    mPlayerRole = role;
}

std::shared_ptr<GameObject> GameSession::GetUserObject() const {
    return mUserObject;
}

uint8_t GameSession::GetSessionState() const {
    return mSessionState.load();
}

Packets::PlayerRole GameSession::GetPlayerRole() const {
    return mPlayerRole;
}
