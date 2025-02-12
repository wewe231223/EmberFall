#include "pch.h"
#include "ViewList.h"
#include "ServerGameScene.h"
#include "GameObject.h"

ViewList::ViewList(SessionIdType ownerId, std::shared_ptr<SessionManager> sessionManager) 
    : mOwnerId{ ownerId }, mSessionManager { sessionManager } { }

ViewList::~ViewList() { }

ViewList& ViewList::operator=(const ViewList& other) {
    return *this;
}

ViewList& ViewList::operator=(ViewList&& other) noexcept {
    return *this;
}

void ViewList::Update() {
    if (nullptr == mCurrentScene) {
        return;
    }

    auto& objectList = mCurrentScene->GetObjects();
    for (const auto& object : objectList) {
        auto objectPos = object->GetPosition();

        float distance = (objectPos - mPosition).LengthSquared();
        if (distance < mViewRange * mViewRange) {
            AddInRange(object);
        }
        else {
            EraseFromRange(object);
        }
    }
}

void ViewList::Send() {
    static size_t prevSendSize{ };

    PacketGameObject objPacket{ sizeof(PacketGameObject), PacketType::PT_GAME_OBJECT_SC };
    for (const auto& object : mObjectInRange) {
        objPacket.objectId = object->GetId() - OBJECT_ID_START;
        objPacket.state = object->IsActive();
        objPacket.color = object->GetColor();
        objPacket.position = object->GetPosition();
        objPacket.rotation = object->GetRotation();
        objPacket.scale = object->GetScale();
        mSessionManager->Send(mOwnerId, &objPacket);
    }

    size_t sendSize{ sizeof(PacketGameObject) * mObjectInRange.size() };
    if (prevSendSize != sendSize) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Send size: {}", sendSize);
        prevSendSize = sendSize;
    }
}

void ViewList::AddInRange(std::shared_ptr<GameObject> obj) {
    mObjectInRange.insert(obj);
}

bool ViewList::EraseFromRange(std::shared_ptr<GameObject> obj) {
    if (false == mObjectInRange.contains(obj)) {
        return false;
    }
   
    mObjectInRange.erase(obj);
}
