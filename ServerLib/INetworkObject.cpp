#include "pch.h"
#include "INetworkObject.h"
#include "NetworkCore.h"

INetworkObject::INetworkObject() { }

INetworkObject::~INetworkObject() { }

void INetworkObject::InitId(NetworkObjectIdType id) {
    mId = id;
}

NetworkObjectIdType INetworkObject::GetId() const {
    return mId;
}