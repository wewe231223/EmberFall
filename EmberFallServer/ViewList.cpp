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

    auto& playerList = mCurrentScene->GetPlayers();
    for (const auto& player : playerList) {
        auto playerPos = player->GetPosition();

        if (MathUtil::IsInRange(mPosition, mViewRange, playerPos)) {
            AddInRange(player);
        }
        else {
            EraseFromRange(player);
        }
    }

    auto& objectList = mCurrentScene->GetObjects();
    for (const auto& object : objectList) {
        auto objectPos = object->GetPosition();

        if (MathUtil::IsInRange(mPosition, mViewRange, objectPos)) {
            AddInRange(object);
        }
        else {
            EraseFromRange(object);
        }
    }
}

void ViewList::Send() {
    static size_t prevSendSize{ };

    PacketPlayerInfoSC playerPacket{ sizeof(PacketPlayerInfoSC), PacketType::PT_PLAYER_INFO_SC };
    PacketGameObject objPacket{ sizeof(PacketGameObject), PacketType::PT_GAME_OBJECT_SC };
    NetworkObjectIdType objectId{ };
    for (const auto& object : mObjectInRange) {
        objectId = object->GetId();
        if (objectId < INVALID_SESSION_ID) {
            playerPacket.id = static_cast<SessionIdType>(objectId);
            playerPacket.color = object->GetColor();
            playerPacket.position = object->GetPosition();
            playerPacket.rotation = object->GetRotation();
            playerPacket.scale = object->GetScale();
            mSessionManager->Send(mOwnerId, &playerPacket);
        }
        else {
            objPacket.objectId = objectId - OBJECT_ID_START;
            objPacket.state = object->IsActive();
            objPacket.color = object->GetColor();
            objPacket.position = object->GetPosition();
            objPacket.rotation = object->GetRotation();
            objPacket.scale = object->GetScale();
            mSessionManager->Send(mOwnerId, &objPacket);
        }
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
    return true;
}

std::set<std::shared_ptr<class GameObject>>& ViewList::GetInRangeObjects() {
    return mObjectInRange;
}
