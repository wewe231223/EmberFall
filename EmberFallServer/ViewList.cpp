#include "pch.h"
#include "ViewList.h"
#include "ServerGameScene.h"
#include "GameObject.h"
#include "GameTimer.h"

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

        if (MathUtil::IsInRange(mPosition, mViewRange, playerPos) and player->mSpec.active) {
            AddInRange(player);
        }
        else {
            EraseFromRange(player);
        }
    }

    auto& objectList = mCurrentScene->GetObjects();
    for (const auto& object : objectList) {
        auto tag = object->GetTag();
        if (ObjectTag::NONE == tag or ObjectTag::TRIGGER == tag or ObjectTag::ENV == tag) {
            return;
        }

        auto objectPos = object->GetPosition();

        if (MathUtil::IsInRange(mPosition, mViewRange, objectPos) and object->mSpec.active) {
            AddInRange(object);
        }
        else {
            EraseFromRange(object);
        }
    }
}

void ViewList::Send() {
    mSendTimeCounter += StaticTimer::GetDeltaTime();
    if (mSendTimeCounter < mSendTimeInterval) {
        return;
    }

    for (const auto& object : mObjectInRange) {
        auto packet = FbsPacketFactory::ObjectMoveSC(
            object->GetId(),
            object->GetEulerRotation().y,
            object->GetPosition(),
            object->GetMoveDir(),
            object->GetSpeed()
        );
        gServerCore->Send(mOwnerId, packet);
    }

    mSendTimeCounter = 0.0f;
}

void ViewList::AddInRange(std::shared_ptr<GameObject> obj) {
    auto ret = mObjectInRange.insert(obj);
    if (false == ret.second) {
        return;
    }

    decltype(auto) packet = FbsPacketFactory::ObjectAppearedSC(
        obj->GetId(),
        obj->mSpec.entity,
        obj->GetEulerRotation().y,
        obj->mAnimationStateMachine.GetCurrState(),
        obj->mSpec.hp,
        obj->GetPosition()
    );
    gServerCore->Send(mOwnerId, packet);
}

bool ViewList::EraseFromRange(std::shared_ptr<GameObject> obj) {
    if (false == mObjectInRange.contains(obj)) {
        return false;
    }
   
    decltype(auto) packet = FbsPacketFactory::ObjectDisappearedSC(obj->GetId());
    gServerCore->Send(mOwnerId, packet);   

    mObjectInRange.erase(obj);

    return true;
}

std::set<std::shared_ptr<class GameObject>>& ViewList::GetInRangeObjects() {
    return mObjectInRange;
}
