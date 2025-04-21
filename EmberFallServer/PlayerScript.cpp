#include "pch.h"
#include "PlayerScript.h"
#include "Input.h"

#include "GameObject.h"
#include "GameEventManager.h"
#include "ObjectManager.h"

#include "GameSession.h"
#include "Sector.h"

#include "Trigger.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input) 
    : Script{ owner, ObjectTag::PLAYER, ScriptType::PLAYER }, mSession{ }, mInput { input }, mViewList{ } {
    owner->mSpec.entity = Packets::EntityType_HUMAN;
    owner->ChangeWeapon(Packets::Weapon_SWORD);
}

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
    auto oldViewList = mViewList;
    mViewListLock.ReadUnlock();

    auto session = mSession.lock();
    if (nullptr == session) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "In UpdateViewListPlayer: std::weak_ptr<GameSession> is null");
        return;
    }

    ViewList newViewList{ };
    for (const auto id : inViewRangePlayer) {
        auto success = newViewList.TryInsert(id);
    }

    for (const auto id : inViewRangeNPC) {
        auto success = newViewList.TryInsert(id);
    }

    for (const auto id : newViewList.GetCurrViewList()) {
        if (not oldViewList.IsInList(id)) {
            decltype(auto) newObj = gObjectManager->GetObjectFromId(id);
            if (nullptr == newObj or false == newObj->mSpec.active) {
                gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "In UpdateViewListPlayer: INVALID Object");
                continue;
            }

            const ObjectSpec spec = newObj->mSpec;
            const auto yaw = newObj->GetEulerRotation().y;
            const auto pos = newObj->GetPosition();
            const auto anim = newObj->mAnimationStateMachine.GetCurrState();
            decltype(auto) packetAppeared = FbsPacketFactory::ObjectAppearedSC(id, spec.entity, yaw, anim, spec.hp, pos);

            session->RegisterSend(packetAppeared);
        }
    }

    for (const auto id : oldViewList.GetCurrViewList()) {
        if (not newViewList.IsInList(id)) {
            decltype(auto) packetDisappeared = FbsPacketFactory::ObjectDisappearedSC(id);

            session->RegisterSend(packetDisappeared);
        }
    }

    mViewListLock.WriteLock();
    mViewList = newViewList;
    mViewListLock.WriteUnlock();
}

void PlayerScript::Init() { 
    //mInteractionTrigger = gObjectSpawner->SpawnTrigger(std::numeric_limits<float>::max(), GetOwner()->GetPosition(), SimpleMath::Vector3{15.0f});
}

void PlayerScript::Update(const float deltaTime) {
    mInput->Update();

    decltype(auto) owner = GetOwner();

    //mInteractionTrigger->GetTransform()->SetPosition(owner->GetPosition());

    // Interact
    if (mInput->IsActiveKey('F')) {
        DoInteraction(deltaTime, GetNearestObject());
    }

    if (mInput->IsUp('F')) {
        CancelInteraction(deltaTime);
    }

    if (true == mInteraction) {
        return;
    }

    // Item
    if (mInput->IsUp('U')) {
        UseItem();
    }

    CheckAndJump(deltaTime);
    CheckAndMove(deltaTime);

    // Attack
    if (mInput->IsActiveKey('P')) {
        owner->Attack();
    }

    gSectorSystem->UpdatePlayerViewList(owner, owner->GetPosition(), mViewList.mViewRange.Count());
}

void PlayerScript::LateUpdate(const float deltaTime) {
}

void PlayerScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->GetPhysics()->SolvePenetration(impulse);
}

void PlayerScript::OnCollisionTerrain(const float height) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (Packets::AnimationState_JUMP == owner->mAnimationStateMachine.GetCurrState()) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    }
}

void PlayerScript::DispatchGameEvent(GameEvent* event) { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto senderTag = gObjectManager->GetObjectFromId(event->sender)->GetTag();
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
    {
        if (event->sender != event->receiver and ObjectTag::PLAYER != senderTag) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            owner->mSpec.hp -= attackEvent->damage;
            owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACKED, true);
            owner->GetPhysics()->AddForce(attackEvent->knockBackForce);

            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[] Attacked!!", owner->GetId());
        }
        break;
    }

    default:
        break;
    }
}

std::shared_ptr<GameObject> PlayerScript::GetNearestObject() {
    //decltype(auto) objects = mInteractionTrigger->GetComponent<Trigger>()->GetObjects();
    //if (objects.empty()) {
    //    return nullptr;
    //}

    //auto owner = GetOwner();
    //auto ownerPos = owner->GetPosition();
    //auto ownerId = owner->GetId();
    //decltype(auto) filterObjects = std::views::filter(objects, [this](NetworkObjectIdType id) {
    //    decltype(auto) obj = mGameScene->GetObjectFromId(id);
    //    if (false == obj->mSpec.interactable or false == obj->mSpec.active) {
    //        return false;
    //    }

    //    return true;
    //    }
    //);

    //if (std::ranges::empty(filterObjects)) {
    //    return nullptr;
    //}

    //if (std::ranges::distance(filterObjects) == 1) {
    //    return mGameScene->GetObjectFromId(*filterObjects.begin());
    //}

    //auto result = *std::ranges::min_element(filterObjects, [&owner, ownerId, ownerPos, this](NetworkObjectIdType id1, NetworkObjectIdType id2) {
    //    if (ownerId == id1 or ownerId == id2) {
    //        return false;
    //    }

    //    decltype(auto) obj1 = mGameScene->GetObjectFromId(id1);
    //    decltype(auto) obj2 = mGameScene->GetObjectFromId(id2);

    //    return SimpleMath::Vector3::DistanceSquared(ownerPos, obj1->GetPosition()) < SimpleMath::Vector3::DistanceSquared(ownerPos, obj2->GetPosition());
    //    }
    //);

    //decltype(auto) resultObject = mGameScene->GetObjectFromId(result);
    return nullptr;
}

void PlayerScript::CheckAndMove(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto currState = owner->mAnimationStateMachine.GetCurrState();
    if (Packets::AnimationState_MOVE_RIGHT < currState) {
        return;
    }

    auto physics{ GetPhysics() };

    SimpleMath::Vector3 moveDir{ SimpleMath::Vector3::Zero };
    if (mInput->IsActiveKey('D')) {
        moveDir.x -= 1.0f;
    }

    if (mInput->IsActiveKey('W')) {
        moveDir.z -= 1.0f;
    }

    if (mInput->IsActiveKey('A')) {
        moveDir.x += 1.0f;
    }

    if (mInput->IsActiveKey('S')) {
        moveDir.z += 1.0f;
    }

    Packets::AnimationState changeState{ Packets::AnimationState_IDLE };
    if (not MathUtil::IsZero(moveDir.x)) {
        physics->mFactor.maxMoveSpeed = 1.5mps;
        changeState = moveDir.x > 0.0f ? Packets::AnimationState_MOVE_LEFT : Packets::AnimationState_MOVE_RIGHT;
    }

    if (not MathUtil::IsZero(moveDir.z)) {
        if (moveDir.z > 0.0f) {
            physics->mFactor.maxMoveSpeed = 1.5mps;
            changeState = Packets::AnimationState_MOVE_BACKWARD;
        }
        else {
            physics->mFactor.maxMoveSpeed = 3.3mps;
            changeState = Packets::AnimationState_MOVE_FORWARD;
        }
    }

    owner->mAnimationStateMachine.ChangeState(changeState);

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, owner->GetTransform()->GetRotation());
    physics->Accelerate(moveDir, owner->GetDeltaTime());
}

void PlayerScript::CheckAndJump(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto physics{ GetPhysics() };

    // Jump
    if (mInput->IsActiveKey(VK_SPACE) and physics->IsOnGround()) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_JUMP);
        physics->CheckAndJump(deltaTime);
    }
}

void PlayerScript::DoInteraction(const float deltaTime, const std::shared_ptr<GameObject>& target) {
    auto owner = GetOwner();
    if (nullptr == owner or nullptr == target or not target->mSpec.active) {
        mInteractionObj = INVALID_OBJ_ID;
        return;
    }

    mInteraction = true;
    if (target->GetId() != mInteractionObj and Packets::EntityType_CORRUPTED_GEM == target->mSpec.entity) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Start");
        auto ownerId = static_cast<SessionIdType>(owner->GetId());
    }

    mInteractionObj = target->GetId();
    switch (target->GetTag()) {
    case ObjectTag::CORRUPTED_GEM:
        DestroyGem(deltaTime, target);
        break;

    case ObjectTag::ITEM:
        AcquireItem(deltaTime, target);
        break;

    default:
        break;
    }
}

void PlayerScript::CancelInteraction(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner or INVALID_OBJ_ID == mInteractionObj or false == mInteraction) {
        return;
    }
    
    mInteraction = false;

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Cancel");
    auto ownerId{ static_cast<SessionIdType>(owner->GetId()) };
    //decltype(auto) packetCancelInteraction = FbsPacketFactory::GemInteractionCancelSC(mInteractionObj, ownerId);
    mInteractionObj = INVALID_OBJ_ID;
}

void PlayerScript::DestroyGem(const float deltaTime, const std::shared_ptr<GameObject>& gem) {
    static float holdStart = 0.0f; // test
    holdStart += deltaTime;

    auto gemId = gem->GetId();
    if (mInteractionObj != gemId) {
        mInteractionObj = gemId;
        holdStart = 0.0f;
    }

    gEventManager->PushEvent<GemDestroyStart>(
        GetOwner()->GetId(),
        gemId,
        holdStart
    );
}

void PlayerScript::AcquireItem(const float deltaTime, const std::shared_ptr<GameObject>& item) {
}

void PlayerScript::UseItem() {

}