#include "pch.h"
#include "ViewList.h"
#include "ServerGameScene.h"
#include "GameObject.h"

ViewList::ViewList(SessionIdType ownerId) 
    : mOwnerId{ ownerId } { }

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

        if (MathUtil::IsInRange(mPosition, mViewRange, playerPos) and player->IsActive()) {
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
    PacketSC::PacketObject objectPacket{
        sizeof(PacketSC::PacketObject),
        PacketType::PACKET_OBJECT,
        mOwnerId
    };

    NetworkObjectIdType objectId{ };
    for (const auto& object : mObjectInRange) {
        objectPacket.objId = object->GetId();
        objectPacket.position = object->GetPosition();
        objectPacket.rotationYaw = object->GetEulerRotation().y;
        gServerCore->Send(mOwnerId, &objectPacket);
    }
}

void ViewList::AddInRange(std::shared_ptr<GameObject> obj) {
    mObjectInRange.insert(obj);

    PacketSC::PacketObjectAppeared objectPacket{
        sizeof(PacketSC::PacketObject),
        PacketType::PACKET_OBJECT_APPEARED,
        mOwnerId
    };

    objectPacket.objId = obj->GetId();
    objectPacket.entity = obj->GetEntityType();
    objectPacket.position = obj->GetPosition();
    objectPacket.rotationYaw = obj->GetEulerRotation().y;
    gServerCore->Send(mOwnerId, &objectPacket);
}

bool ViewList::EraseFromRange(std::shared_ptr<GameObject> obj) {
    if (false == mObjectInRange.contains(obj)) {
        return false;
    }
   
    PacketSC::PacketObjectDisappeared objectPacket{
        sizeof(PacketSC::PacketObject),
        PacketType::PACKET_OBJECT_DISAPPEARED,
        mOwnerId
    };

    objectPacket.objId = obj->GetId();
    gServerCore->Send(mOwnerId, &objectPacket);   

    mObjectInRange.erase(obj);

    return true;
}

std::set<std::shared_ptr<class GameObject>>& ViewList::GetInRangeObjects() {
    return mObjectInRange;
}
