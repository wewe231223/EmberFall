#include "pch.h"
#include "INetworkObject.h"
#include "NetworkCore.h"
#include "OverlappedEx.h"

INetworkObject::INetworkObject() { }

INetworkObject::~INetworkObject() { }

void INetworkObject::InitId(NetworkObjectIdType id) {
    mId = id;
}

NetworkObjectIdType INetworkObject::GetId() const {
    return mId;
}

void INetworkObject::StorePacket(OverlappedSend* sendBuf) {
    mSendBuf.push(sendBuf);
}

Concurrency::concurrent_queue<OverlappedSend*>& INetworkObject::GetSendBuf() {
    return mSendBuf;
}
