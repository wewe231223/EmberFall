#include "pch.h"
#include "HumanPlayerScript.h"
#include "Input.h"

#include "GameObject.h"
#include "ObjectManager.h"
#include "ServerFrame.h"

#include "GameSession.h"
#include "Sector.h"

#include "Trigger.h"
#include "GameRoom.h"

#include "BuffHealScript.h"
#include "ItemScript.h"

HumanPlayerScript::HumanPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input) 
    : PlayerScript{ owner, input, ObjectTag::PLAYER, ScriptType::PLAYER } {
    owner->mSpec.entity = Packets::EntityType_HUMAN;
}

HumanPlayerScript::~HumanPlayerScript() { }

bool HumanPlayerScript::IsAttackableBoss() {
    return mAttackableBoss;
}

void HumanPlayerScript::Init() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.moveable = true;
    spec.interactable = false;
    spec.animated = true;
    spec.damage = 30.0f;
    spec.defence = 0.0f;
    owner->mSpec.hp = GameProtocol::Logic::MAX_HP;

    const auto pos = owner->GetPosition();
    const auto look = owner->GetTransform()->Forward();
    auto ownerRoom = owner->GetMyRoomIdx();
    mInteractionTrigger = gGameRoomManager->GetRoom(ownerRoom)->GetStage().SpawnTrigger(
        pos, SimpleMath::Vector3{ 1.0f, 3.0f, 1.0f }, look, std::numeric_limits<float>::max());
}

void HumanPlayerScript::Update(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    mInteractionTrigger->GetTransform()->SetPosition(owner->GetPosition());
    mInteractionTrigger->GetTransform()->SetLook(owner->GetTransform()->Forward());
    mInteractionTrigger->Update();

    if (mInput->IsDown(VK_F1)) {
        mSuperMode = !mSuperMode;
    }

    if (mInput->IsDown(VK_F4)) {
        Suicide();
    }

    // Item
    if (mInput->IsDown(GameProtocol::Key::KEY_USE_ITEM)) {
        UseItem();
    }

    // Interact
    if (mInput->IsActiveKey(GameProtocol::Key::KEY_INTERACTION)) {
        auto interactTarget = GetNearestObject();
        DoInteraction(interactTarget);
    } else {
        CancelInteraction();
    }

    if (true == mInteraction) {
        return;
    }

    CheckAndJump(deltaTime);
    CheckAndMove(deltaTime);

    mInput->Update();

    auto ownerRoom = owner->GetMyRoomIdx();
    gGameRoomManager->GetRoom(ownerRoom)->GetStage().UpdatePlayerViewList(owner, owner->GetPosition(), GetViewList().mViewRange.Count());
}

void HumanPlayerScript::LateUpdate(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    mInteractionTrigger->LateUpdate();

    if (owner->mSpec.hp > MathUtil::EPSILON) {
        return;
    }

    auto isDead = owner->IsDead();
    if (not isDead) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_DEAD);
    }

    if (isDead and owner->mAnimationStateMachine.GetRemainDuration() <= 0.0f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Monster Remove");
        auto executionTime = SysClock::now();
        gServerFrame->AddTimerEvent(owner->GetMyRoomIdx(), owner->GetId(), executionTime, TimerEventType::REMOVE_NPC);
        owner->mSpec.active = false;
        return;
    }
}

void HumanPlayerScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { 
    auto tag = opponent->GetTag();
    if (ObjectTag::TRIGGER == tag or ObjectTag::ITEM == tag) {
        return;
    }

    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->GetPhysics()->SolvePenetration(impulse);
}

void HumanPlayerScript::OnCollisionTerrain(const float height) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (Packets::AnimationState_JUMP == owner->mAnimationStateMachine.GetCurrState()) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    }
}

void HumanPlayerScript::DispatchGameEvent(GameEvent* event) { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    auto sender = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(event->sender);
    if (nullptr == sender) {
        return;
    }

    auto senderTag = sender->GetTag();
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
    {
        if (event->sender != event->receiver and ObjectTag::PLAYER != senderTag) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            owner->mSpec.hp -= attackEvent->damage;
            owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACKED, true);
            owner->GetPhysics()->AddForce(attackEvent->knockBackForce, owner->GetDeltaTime());

            CancelInteraction();

            auto packetAttacked = FbsPacketFactory::ObjectAttackedSC(owner->GetId(), owner->mSpec.hp);
            owner->StorePacket(packetAttacked);
        }
        break;
    }

    case GameEventType::DESTROY_GEM_COMPLETE:
    {
        SuccessInteraction();
        break;
    }

    default:
        break;
    }
}

std::shared_ptr<GameObject> HumanPlayerScript::GetNearestObject() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return nullptr;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    if (true == mInteraction) {
        return gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(mInteractionObj);
    }

    decltype(auto) objects = mInteractionTrigger->GetScript<Trigger>()->GetObjects();
    if (objects.empty()) {
        return nullptr;
    }

    auto ownerPos = owner->GetPosition();
    auto ownerId = owner->GetId();
    decltype(auto) filterObjects = std::views::filter(objects, [this, ownerRoom](NetworkObjectIdType id) {
        decltype(auto) obj = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(id);
        if (nullptr == obj or false == obj->mSpec.interactable or false == obj->mSpec.active) {
            return false;
        }

        return true;
        }
    );

    if (std::ranges::empty(filterObjects)) {
        return nullptr;
    }

    if (std::ranges::distance(filterObjects) == 1) {
        return gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(*filterObjects.begin());
    }

    auto result = *std::ranges::min_element(filterObjects, [&owner, ownerId, ownerPos, ownerRoom, this](NetworkObjectIdType id1, NetworkObjectIdType id2) {
        if (ownerId == id1 or ownerId == id2) {
            return false;
        }

        decltype(auto) obj1 = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(id1);
        decltype(auto) obj2 = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(id2);

        return SimpleMath::Vector3::DistanceSquared(ownerPos, obj1->GetPosition()) < SimpleMath::Vector3::DistanceSquared(ownerPos, obj2->GetPosition());
        }
    );

    decltype(auto) resultObject = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(result);
    return resultObject;
}

void HumanPlayerScript::CheckAndMove(const float deltaTime) {
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
    for (const auto& [key, dir] : GameProtocol::Key::KEY_MOVE_DIR) {
        if (mInput->IsActiveKey(key)) {
            moveDir += dir;
        }
    }

    Packets::AnimationState changeState{ Packets::AnimationState_IDLE };
    if (not MathUtil::IsZero(moveDir.z)) {
        if (not mSuperMode) {
            if (moveDir.z > 0.0f) {
                physics->mFactor.maxMoveSpeed = GameProtocol::Unit::PLAYER_WALK_SPEED;
                changeState = Packets::AnimationState_MOVE_BACKWARD;
            }
            else {
                physics->mFactor.maxMoveSpeed = GameProtocol::Unit::PLAYER_RUN_SPEED;
                changeState = Packets::AnimationState_MOVE_FORWARD;
            }
        }
        else {
            if (moveDir.z > 0.0f) {
                physics->mFactor.maxMoveSpeed = mSuperSpeed;
                changeState = Packets::AnimationState_MOVE_BACKWARD;
            }
            else {
                physics->mFactor.maxMoveSpeed = mSuperSpeed;
                changeState = Packets::AnimationState_MOVE_FORWARD;
            }
        }
    }
    else if (not MathUtil::IsZero(moveDir.x)) {
        physics->mFactor.maxMoveSpeed = GameProtocol::Unit::PLAYER_WALK_SPEED;
        changeState = moveDir.x > 0.0f ? Packets::AnimationState_MOVE_LEFT : Packets::AnimationState_MOVE_RIGHT;
    }

    owner->mAnimationStateMachine.ChangeState(changeState);

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, owner->GetTransform()->GetRotation());
    physics->Accelerate(moveDir, owner->GetDeltaTime());
}

void HumanPlayerScript::CheckAndJump(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto physics{ GetPhysics() };

    // Jump
    if (mInput->IsActiveKey(GameProtocol::Key::KEY_JUMP) and physics->IsOnGround()) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_JUMP);
        physics->CheckAndJump(deltaTime);
    }
}

void HumanPlayerScript::DoInteraction(const std::shared_ptr<GameObject>& target) {
    auto owner = GetOwner();
    if (nullptr == owner or nullptr == target or not target->mSpec.active) {
        mInteractionObj = INVALID_OBJ_ID;
        return;
    }

    auto isAttacked = owner->mAnimationStateMachine.GetCurrState() == Packets::AnimationState_ATTACKED;
    if (isAttacked) {
        CancelInteraction();
        return;
    }

    auto deltaTime = owner->GetDeltaTime();
    mInteraction = true;
    if (target->GetId() != mInteractionObj) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_INTERACTION, true);
    }

    mInteractionObj = target->GetId();
    switch (target->GetTag()) {
    case ObjectTag::CORRUPTED_GEM:
        DestroyingGem(deltaTime, target);
        break;

    case ObjectTag::ITEM:
        AcquireItem(deltaTime, target);
        mInput->ChangeKeyState(GameProtocol::Key::KEY_INTERACTION, false);
        break;

    default:
        break;
    }
}

void HumanPlayerScript::CancelInteraction() {
    auto owner = GetOwner();
    if (false == mInteraction or INVALID_OBJ_ID == mInteractionObj or nullptr == owner) {
        mInteraction = false;
        return;
    }

    auto ownerId = owner->GetId();
    auto ownerRoom = owner->GetMyRoomIdx();
    auto obj = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(mInteractionObj);
    if (nullptr == obj) {
        mInteractionObj = INVALID_OBJ_ID;
        mInteraction = false;
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE, true);
        return;
    }

    auto eventCancel = GameEventFactory::GetEvent<DestroyingGemCancel>(ownerId, mInteractionObj);
    obj->DispatchGameEvent(eventCancel);

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE, true);
    mInteractionObj = INVALID_OBJ_ID;
    mInteraction = false;
}

void HumanPlayerScript::DestroyingGem(const float deltaTime, const std::shared_ptr<GameObject>& gem) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }
    
    auto gemId = gem->GetId();
    if (mInteractionObj != gemId) {
        mInteractionObj = gemId;
    }

    auto event = GameEventFactory::GetEvent<DestroyingGemEvent>(owner->GetId(), gemId, deltaTime);
    gem->DispatchGameEvent(event);
}

void HumanPlayerScript::SuccessInteraction() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE, true);
    mInteractionObj = INVALID_OBJ_ID;
    mInteraction = false;
}

void HumanPlayerScript::AcquireItem(const float deltaTime, const std::shared_ptr<GameObject>& item) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto itemScript = item->GetScript<ItemScript>();
    if (nullptr == itemScript) {
        return;
    }

    auto itemTag = itemScript->GetItemTag();
    auto idx = mInventory.AcquireItem(itemTag);
    if (0xFF == idx) {
        SuccessInteraction();
        return;
    }

    auto packetAcquire = FbsPacketFactory::AcquireItemSC(static_cast<SessionIdType>(owner->GetId()), idx, ItemTagToItemType(itemTag));
    gServerCore->Send(static_cast<SessionIdType>(owner->GetId()), packetAcquire);

    auto executionTime = SysClock::now();
    gServerFrame->AddTimerEvent(owner->GetMyRoomIdx(), item->GetId(), executionTime, TimerEventType::REMOVE_NPC);

    SuccessInteraction();
}

void HumanPlayerScript::UseItem() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    mInventory.UseItem(owner);
}

void HumanPlayerScript::NotifyAllOfGemDestroyed() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Now Player: {} Can Attack Boss!", owner->GetId());
    mAttackableBoss = true;
}
