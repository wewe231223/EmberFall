#include "pch.h"
#include "INetworkObject.h"
#include "NetworkCore.h"
#include "OverlappedEx.h"

INetworkObject::INetworkObject() { }

INetworkObject::INetworkObject(uint16_t gameRoomIdx) 
    : mGameRoomIdx{ gameRoomIdx } { }

INetworkObject::~INetworkObject() { }

void INetworkObject::InitId(NetworkObjectIdType id) {
    mId = id;
}

NetworkObjectIdType INetworkObject::GetId() const {
    return mId;
}

uint16_t INetworkObject::GetMyRoomIdx() const {
    return mGameRoomIdx;
}

void INetworkObject::SetRoomIdx(uint16_t roomIdx) {
    mGameRoomIdx = roomIdx;
}

void INetworkObject::StorePacket(OverlappedSend* sendBuf) {
    mSendBuf.push(sendBuf);
}

Concurrency::concurrent_queue<OverlappedSend*>& INetworkObject::GetSendBuf() {
    return mSendBuf;
}
