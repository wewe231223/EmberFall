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
    for (const auto& player : playerList | std::views::filter([](const std::shared_ptr<GameObject>& object) { return object->IsActive(); })) {
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

        if (MathUtil::IsInRange(mPosition, mViewRange, objectPos) and object->IsActive()) {
            AddInRange(object);
        }
        else {
            EraseFromRange(object);
        }
    }
}

void ViewList::Send() {
    static size_t prevSendSize{ };

    PacketSC::PacketPlayer playerPacket{
        sizeof(PacketSC::PacketPlayer),
        PacketType::PACKET_PLAYER,
    };

    PacketSC::PacketObject objectPacket{ 
        sizeof(PacketSC::PacketObject), 
        PacketType::PACKET_OBJECT,
        mOwnerId
    };

    NetworkObjectIdType objectId{ };
    for (const auto& object : mObjectInRange) {
        objectId = object->GetId();
        if (objectId < INVALID_SESSION_ID) {
            playerPacket.id = static_cast<SessionIdType>(objectId);
            playerPacket.position = object->GetPosition();
            playerPacket.rotationYaw = object->GetEulerRotation().y;
            mSessionManager->Send(mOwnerId, &playerPacket);
        }
        else {
            objectPacket.objId = objectId - OBJECT_ID_START;
            objectPacket.position = object->GetPosition();
            objectPacket.rotationYaw = object->GetEulerRotation().y;
            mSessionManager->Send(mOwnerId, &objectPacket);
        }

    }
}

void ViewList::AddInRange(std::shared_ptr<GameObject> obj) {
    mObjectInRange.insert(obj);

    //PacketSC::PacketObjectAppeared objectPacket{
    //    sizeof(PacketSC::PacketObject),
    //    PacketType::PACKET_OBJECT,
    //    mOwnerId
    //};

    //objectPacket.objId = obj->GetId() - OBJECT_ID_START;
    //objectPacket.entity = obj->GetEntityType();
    //objectPacket.position = obj->GetPosition();
    //objectPacket.rotationYaw = obj->GetEulerRotation().y;
    //mSessionManager->Send(mOwnerId, &objectPacket);
}

bool ViewList::EraseFromRange(std::shared_ptr<GameObject> obj) {
    if (false == mObjectInRange.contains(obj)) {
        return false;
    }
   
    //PacketSC::PacketObjectDisappeared objectPacket{
    //    sizeof(PacketSC::PacketObject),
    //    PacketType::PACKET_OBJECT,
    //    mOwnerId
    //};

    //objectPacket.objId = obj->GetId() - OBJECT_ID_START;
    //mSessionManager->Send(mOwnerId, &objectPacket);   

    mObjectInRange.erase(obj);

    return true;
}

std::set<std::shared_ptr<class GameObject>>& ViewList::GetInRangeObjects() {
    return mObjectInRange;
}
