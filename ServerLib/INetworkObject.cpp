#include "pch.h"
#include "INetworkObject.h"
#include "NetworkCore.h"

INetworkObject::INetworkObject(std::shared_ptr<INetworkCore> coreService) 
    : mCoreService{ coreService } { }

INetworkObject::~INetworkObject() { }

void INetworkObject::InitId(SessionIdType id) {
    mId = id;
}

SessionIdType INetworkObject::GetId() const {
    return mId;
}

std::shared_ptr<INetworkCore> INetworkObject::GetCore() const {
    return mCoreService;
}