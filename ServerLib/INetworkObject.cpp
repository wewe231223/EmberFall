#include "pch.h"
#include "INetworkObject.h"
#include "IOCPCore.h"

INetworkObject::INetworkObject() { }

INetworkObject::~INetworkObject() { }

SessionIdType INetworkObject::GetId() const {
    return mId;
}

void INetworkObject::PostQueuedCompletionStatus() {
    bool result = ::PostQueuedCompletionStatus(gIOCPCore.GetHandle(), 0, 0, nullptr);
    CrashExp(false != result, "PQCS Failed");
}
