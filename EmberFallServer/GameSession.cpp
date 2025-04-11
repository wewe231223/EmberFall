#include "pch.h"
#include "GameSession.h"
#include "FbsPacketProcessFn.h"
#include "PlayerScript.h"
#include "ObjectManager.h"

GameSession::GameSession(std::shared_ptr<INetworkCore> core) 
    : Session{ core } { 
}

GameSession::~GameSession() { 
    Session::Close();
}

void GameSession::OnConnect() {
    mUserObject = gObjectManager->GetObjectFromId(GetId());
    mUserObject->mSpec.active = true;
    auto sharedFromThis = std::static_pointer_cast<GameSession>(shared_from_this());
    auto player = mUserObject->GetScript<PlayerScript>();
    if (nullptr == player) {}

    player->SetOwnerSession(sharedFromThis);
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

std::shared_ptr<GameObject> GameSession::GetUserObject() const {
    return mUserObject;
}
