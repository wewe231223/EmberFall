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

    PacketSC::PacketObject objectPacket{ 
        sizeof(PacketSC::PacketObject), 
        PacketType::PACKET_OBJECT,
        mOwnerId
    };

    NetworkObjectIdType objectId{ };
    for (const auto& object : mObjectInRange) {
        objectId = object->GetId();
        objectPacket.position = object->GetPosition();
        //objectPacket.rotationYaw = ;

        if (objectId < INVALID_SESSION_ID) {
            objectPacket.objId = objectId;
        }
        else {
            objectPacket.objId = objectId - OBJECT_ID_START;
        }

        mSessionManager->Send(mOwnerId, &objectPacket);
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
