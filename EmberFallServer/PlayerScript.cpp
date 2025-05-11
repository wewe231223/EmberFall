#include "pch.h"
#include "PlayerScript.h"

#include "Input.h"

#include "GameObject.h"
#include "GameSession.h"
#include "ServerFrame.h"

#include "GameRoom.h"
#include "Sector.h"
#include "ObjectManager.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input, ObjectTag tag, ScriptType scType)
    : Script{ owner, tag, scType }, mInput{ input } { }

PlayerScript::~PlayerScript() { }

std::shared_ptr<Input> PlayerScript::GetInput() const {
    return mInput;
}

ViewList& PlayerScript::GetViewList() {
    return mViewList;
}

void PlayerScript::SetOwnerSession(std::shared_ptr<class GameSession> session) {
    mSession = session;
    mViewList.TryInsert(session->GetId());
}

void PlayerScript::UpdateViewList(const std::vector<NetworkObjectIdType>& inViewRangeNPC, const std::vector<NetworkObjectIdType>& inViewRangePlayer) {
    mViewListLock.ReadLock();
    std::unordered_set<NetworkObjectIdType> oldViewList = mViewList.GetCurrViewList();
    mViewListLock.ReadUnlock();

    auto session = mSession.lock();
    if (nullptr == session) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "In UpdateViewListPlayer: std::weak_ptr<GameSession> is null");
        return;
    }

    if (SESSION_INGAME != std::static_pointer_cast<GameSession>(session)->GetSessionState()) {
        return;
    }
    
    decltype(auto) myRoomStage = gGameRoomManager->GetRoom(session->GetMyRoomIdx())->GetStage();

    ViewList newViewList{ };
    for (const auto id : inViewRangePlayer) {
        auto success = newViewList.TryInsert(id);
        if (not success) {
            continue;
        }
    }

    for (const auto id : inViewRangeNPC) {
        auto success = newViewList.TryInsert(id);
        if (not success) {
            continue;
        }
    }

    auto ownerRoom = session->GetMyRoomIdx();
    decltype(auto) roomStage = gGameRoomManager->GetRoom(ownerRoom)->GetStage();
    for (const auto id : newViewList.GetCurrViewList()) {
        if (oldViewList.contains(id)) {
            continue;
        }

        decltype(auto) newObj = roomStage.GetObjectFromId(id);
        if (nullptr == newObj or false == newObj->mSpec.active) {
            continue;
        }

        const ObjectSpec spec = newObj->mSpec;
        const auto yaw = newObj->GetEulerRotation().y;
        const auto pos = newObj->GetPosition();
        const auto anim = newObj->mAnimationStateMachine.GetCurrState();

        decltype(auto) packetAppeared = FbsPacketFactory::ObjectAppearedSC(id, spec.entity, yaw, anim, spec.hp, pos);
        session->RegisterSend(packetAppeared);
    }

    for (const auto id : oldViewList) {
        if (not newViewList.IsInList(id)) {
            decltype(auto) packetDisappeared = FbsPacketFactory::ObjectDisappearedSC(id);
            session->RegisterSend(packetDisappeared);
        }
    }

    mViewListLock.WriteLock();
    mViewList = newViewList;
    mViewListLock.WriteUnlock();
}
