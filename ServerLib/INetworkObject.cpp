#include "pch.h"
#include "INetworkObject.h"
#include "NetworkCore.h"
#include "OverlappedEx.h"

IServerEntity::IServerEntity() { }

IServerEntity::IServerEntity(uint16_t gameRoomIdx) 
    : mGameRoomIdx{ gameRoomIdx } { }

IServerEntity::~IServerEntity() { }

void IServerEntity::InitId(NetworkObjectIdType id) {
    mId = id;
}

NetworkObjectIdType IServerEntity::GetId() const {
    return mId;
}

uint16_t IServerEntity::GetMyRoomIdx() const {
    return mGameRoomIdx;
}

void IServerEntity::SetRoomIdx(uint16_t roomIdx) {
    mGameRoomIdx = roomIdx;
}

void IServerEntity::StorePacket(OverlappedSend* sendBuf) {
    mSendBuf.push(sendBuf);
}

Concurrency::concurrent_queue<OverlappedSend*>& IServerEntity::GetSendBuf() {
    return mSendBuf;
}
