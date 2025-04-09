#include "pch.h"
#include "PlayerScript.h"
#include "Input.h"

#include "GameObject.h"
#include "ServerGameScene.h"
#include "GameEventManager.h"
#include "ObjectSpawner.h"

#include "Trigger.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input)
    : Script{ owner, ObjectTag::PLAYER }, mInput{ input }, mViewList{ static_cast<SessionIdType>(owner->GetId()) } {
    owner->SetEntityType(EntityType::PLAYER);
    owner->ChangeWeapon(Weapon::SWORD);
}

PlayerScript::~PlayerScript() { }

void PlayerScript::ResetGameScene(std::shared_ptr<IServerGameScene> gameScene) {
    mGameScene = gameScene;
    mViewList.mCurrentScene = gameScene;
}

std::shared_ptr<IServerGameScene> PlayerScript::GetCurrentScene() const {
    return mGameScene;
}

void PlayerScript::Init() { 
    auto triggerBoxExt = SimpleMath::Vector3{ 1.0f, 1.0f, 5.0f };
    mInteractionTrigger = gObjectSpawner->SpawnTrigger(std::numeric_limits<float>::max(), GetOwner()->GetPosition(), triggerBoxExt);
}

void PlayerScript::Update(const float deltaTime) {
    decltype(auto) owner = GetOwner();

    mInteractionTrigger->GetTransform()->SetPosition(owner->GetPosition());
    mInteractionTrigger->GetTransform()->Rotation(owner->GetRotation());
    mViewList.mPosition = owner->GetPosition();
    mViewList.Update();
    mViewList.Send();

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
    if (mInput->IsUp('P')) {
        owner->Attack();
    }
}

void PlayerScript::LateUpdate(const float deltaTime) {
    mInput->Update();
}

void PlayerScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void PlayerScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (AnimationState::DEAD == opponent->mAnimationStateMachine.GetCurrState() or AnimationState::DEAD == GetOwner()->mAnimationStateMachine.GetCurrState()) {
        return;
    }

    switch (opponent->GetTag()) {
    case ObjectTag::MONSTER:
        GetOwner()->GetPhysics()->SolvePenetration(impulse, opponent);
        break;

    case ObjectTag::PLAYER:
        GetOwner()->GetPhysics()->SolvePenetration(impulse, opponent);
        break;

    case ObjectTag::CORRUPTED_GEM:
        GetOwner()->GetPhysics()->SolvePenetration(impulse, opponent);
        break;

    default:
        break;
    }
}

void PlayerScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void PlayerScript::OnCollisionTerrain(const float height) {
    if (AnimationState::JUMP == GetOwner()->mAnimationStateMachine.GetCurrState()) {
        GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::IDLE);
    }
}

void PlayerScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        if (event->sender != event->receiver) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            GetOwner()->ReduceHealth(attackEvent->damage);
            GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::ATTACKED, true);
            GetOwner()->GetPhysics()->AddForce(attackEvent->knockBackForce);

            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[] Attacked!!", GetOwner()->GetId());
        }
        break;

    case GameEventType::DESTROY_GEM_COMPLETE:
        {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Finish");
            auto ownerId = static_cast<SessionIdType>(GetOwner()->GetId());
            auto packet = GetPacket<PacketSC::PacketInteractFinished>(
                ownerId,
                EntityType::CORRUPTED_GEM,
                ownerId,
                event->sender
            );

            gServerCore->Send(ownerId, &packet);
            mInteraction = false;
            mInteractionObj = INVALID_OBJ_ID;
        }
        break;
    }
}

std::shared_ptr<GameObject> PlayerScript::GetNearestObject() {
    decltype(auto) objects = mInteractionTrigger->GetComponent<Trigger>()->GetObjects();
    if (objects.empty()) {
        return nullptr;
    }

    auto owner = GetOwner();
    auto ownerPos = owner->GetPosition();
    auto ownerId = owner->GetId();
    decltype(auto) filterObjects = std::views::filter(objects, [this](NetworkObjectIdType id) {
        decltype(auto) obj = mGameScene->GetObjectFromId(id);
        if (false == obj->IsInteractable() or false == obj->IsActive()) {
            return false;
        }

        return true;
        }
    );

    if (std::ranges::empty(filterObjects)) {
        return nullptr;
    }

    if (std::ranges::distance(filterObjects) == 1) {
        return mGameScene->GetObjectFromId(*filterObjects.begin());
    }

    auto result = *std::ranges::min_element(filterObjects, [&owner, ownerId, ownerPos, this](NetworkObjectIdType id1, NetworkObjectIdType id2) {
        if (ownerId == id1 or ownerId == id2) {
            return false;
        }

        decltype(auto) obj1 = mGameScene->GetObjectFromId(id1);
        decltype(auto) obj2 = mGameScene->GetObjectFromId(id2);

        return SimpleMath::Vector3::DistanceSquared(ownerPos, obj1->GetPosition()) < SimpleMath::Vector3::DistanceSquared(ownerPos, obj2->GetPosition());
        }
    );

    decltype(auto) resultObject = mGameScene->GetObjectFromId(result);
    return resultObject;
}

void PlayerScript::CheckAndMove(const float deltaTime) {
    auto currState = GetOwner()->mAnimationStateMachine.GetCurrState();
    if (AnimationState::MOVE_RIGHT < currState) {
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

    AnimationState changeState{ AnimationState::IDLE };
    if (not MathUtil::IsZero(moveDir.x)) {
        physics->mFactor.maxMoveSpeed = 1.5mps;
        changeState = moveDir.x > 0.0f ? AnimationState::MOVE_LEFT : AnimationState::MOVE_RIGHT;
    }

    if (not MathUtil::IsZero(moveDir.z)) {
        if (moveDir.z > 0.0f) {
            physics->mFactor.maxMoveSpeed = 1.5mps;
            changeState = AnimationState::MOVE_BACKWARD;
        }
        else {
            physics->mFactor.maxMoveSpeed = 3.3mps;
            changeState = AnimationState::MOVE_FORWARD;
        }
    }

    GetOwner()->mAnimationStateMachine.ChangeState(changeState);

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, GetOwner()->GetTransform()->GetRotation());
    physics->Accelerate(moveDir);
}

void PlayerScript::CheckAndJump(const float deltaTime) {
    auto physics{ GetPhysics() };

    // Jump
    if (mInput->IsDown(VK_SPACE) and physics->IsOnGround()) {
        GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::JUMP);
        physics->CheckAndJump(deltaTime);
    }
}

void PlayerScript::DoInteraction(const float deltaTime, const std::shared_ptr<GameObject>& target) {
    if (nullptr == target or not target->IsActive()) {
        mInteractionObj = INVALID_OBJ_ID;
        return;
    }

    mInteraction = true;
    if (target->GetId() != mInteractionObj and EntityType::CORRUPTED_GEM == target->GetEntityType()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Start");
        auto ownerId = static_cast<SessionIdType>(GetOwner()->GetId());
        auto packet = GetPacket<PacketSC::PacketInteractStart>(
            ownerId, 
            target->GetEntityType(), 
            ownerId, 
            target->GetId()
        );

        gServerCore->Send(ownerId, &packet);
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
    if (INVALID_OBJ_ID == mInteractionObj or false == mInteraction) {
        return;
    }
    
    mInteraction = false;

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Cancel");
    auto ownerId{ static_cast<SessionIdType>(GetOwner()->GetId()) };
    auto packet = GetPacket<PacketSC::PacketInteractCancel>(
        ownerId,
        ownerId
    );

    gServerCore->Send(ownerId, &packet);
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