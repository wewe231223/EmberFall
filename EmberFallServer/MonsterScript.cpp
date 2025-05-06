#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"
#include "BehaviorTreeBase.h"
#include "ObjectManager.h"
#include "ServerFrame.h"
#include "GameRoom.h"
#include "Sector.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner, ObjectTag::MONSTER, ScriptType::MONSTER } {
    owner->mSpec.entity = Packets::EntityType_MONSTER;
}

MonsterScript::~MonsterScript() { }

void MonsterScript::Init() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->mAnimationStateMachine.Init(ANIM_KEY_MONSTER);
    auto& spec = owner->mSpec;
    spec.active = true;
    spec.hp = 10.0f;

    mMonsterBT.Build(std::static_pointer_cast<MonsterScript>(shared_from_this()));
    owner->GetPhysics()->mFactor.maxMoveSpeed = 1.5mps;
}

void MonsterScript::Update(const float deltaTime) {
    mMonsterBT.Update(deltaTime);
}

void MonsterScript::LateUpdate(const float deltaTime) { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (owner->mSpec.hp > MathUtil::EPSILON) {
        return;
    }

    auto isDead = owner->IsDead();
    if (not isDead) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_DEAD);
    }

    if (isDead and owner->mAnimationStateMachine.GetRemainDuration() <= 0.0f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "GameRoom [{}]: Remove Monster {}", owner->GetMyRoomIdx(), owner->GetId());
        gServerFrame->AddTimerEvent(owner->GetMyRoomIdx(), owner->GetId(), SysClock::now(), TimerEventType::REMOVE_NPC);
        owner->mSpec.active = false;

        auto packetRemove = FbsPacketFactory::ObjectRemoveSC(owner->GetId());
        owner->StorePacket(packetRemove);
        return;
    }
}

void MonsterScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (ObjectTag::TRIGGER == opponent->GetTag()) {
        return;
    }

    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->GetPhysics()->SolvePenetration(impulse);
}

void MonsterScript::OnCollisionTerrain(const float height) { }

void MonsterScript::DispatchGameEvent(GameEvent* event) { 
    auto owner = GetOwner();
    if (nullptr == owner or owner->IsDead()) {
        return;
    }
    
    auto ownerRoom = owner->GetMyRoomIdx();
    auto senderTag = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(event->sender)->GetTag();
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
    {
        if (event->sender != event->receiver and ObjectTag::MONSTER != senderTag and ObjectTag::BOSSPLAYER != senderTag) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            owner->mSpec.hp -= attackEvent->damage;

            owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACKED, true);

            auto packetAttacked = FbsPacketFactory::ObjectAttackedSC(owner->GetId(), owner->mSpec.hp);
            owner->StorePacket(packetAttacked);

            mMonsterBT.Interrupt();
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Monster [{}] Attacked!!, HP: {}", owner->GetId(), owner->mSpec.hp);
        }
        break;
    }

    default:
        break;
    }
}

NetworkObjectIdType MonsterScript::GetChaseTarget() const {
    return mChaseTarget;
}

bool MonsterScript::IsPlayerInAttackRange() const {
    if (INVALID_OBJ_ID == mChaseTarget) {
        return false;
    }

    auto owner = GetOwner();
    if (nullptr == owner) {
        return false;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    auto chaseTarget = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetPlayer(mChaseTarget);
    if (nullptr == chaseTarget or false == chaseTarget->mSpec.active) {
        return false;
    }

    auto targetPos = chaseTarget->GetPosition();
    auto moveDir = targetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    targetPos.y = 0.0f;
    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;

    float attackRange = mAttackRange.Count();
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) > MathUtil::Square(attackRange)) {
        return false;
    }

    return true;
}

BT::NodeStatus MonsterScript::SetRandomTargetLocation(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return BT::NodeStatus::FAIL;
    }

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    mTargetPos = Random::GetRandomVec3(SimpleMath::Vector3{ -100.0f, 0.0f, -100.0f }, SimpleMath::Vector3{ 100.0f, 0.0f, 100.0f });
    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus MonsterScript::MoveTo(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return BT::NodeStatus::FAIL;
    }

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_MOVE_FORWARD);

    auto transform = owner->GetTransform();

    auto moveDir = mTargetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;
    if (SimpleMath::Vector3::DistanceSquared(mTargetPos, compareXZ) < 0.5f * 0.5f) {
        return BT::NodeStatus::SUCCESS;
    }

    owner->GetTransform()->SetLook(moveDir);
    owner->GetPhysics()->Accelerate(moveDir, owner->GetDeltaTime());
    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MonsterScript::DetectPlayerInRange(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return BT::NodeStatus::FAIL;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    std::vector<NetworkObjectIdType> players = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetNearbyPlayers(owner->GetPosition(), 10.0f);
    for (const auto& playerId : players) {
        auto playerObj = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetPlayer(playerId);
        if (nullptr == playerObj or false == playerObj->mSpec.active) {
            continue;
        }

        auto distSq = SimpleMath::Vector3::DistanceSquared(playerObj->GetPosition(), owner->GetPosition());
        if (distSq < MathUtil::Square(mPlayerDetectRange.Count())) {
            mChaseTarget = playerId;
            return BT::NodeStatus::SUCCESS;
        }
    }

    return BT::NodeStatus::FAIL;
}

BT::NodeStatus MonsterScript::ChaseDetectedPlayer(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return BT::NodeStatus::FAIL;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    auto chaseTarget = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetPlayer(mChaseTarget);
    if (nullptr == chaseTarget or false == chaseTarget->mSpec.active) {
        BT::NodeStatus::FAIL;
    }

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_MOVE_FORWARD);
    auto targetPos = chaseTarget->GetPosition();
    auto moveDir = targetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    targetPos.y = 0.0f;
    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) < MathUtil::Square(mAttackRange.Count())) {
        return BT::NodeStatus::SUCCESS;
    }
    
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) > MathUtil::Square(mPlayerDetectRange.Count())) {
        return BT::NodeStatus::FAIL;
    }

    owner->GetTransform()->SetLook(moveDir);
    owner->GetPhysics()->Accelerate(moveDir, GetOwner()->GetDeltaTime());
    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MonsterScript::CheckPlayerInAttackRange(const float deltaTime) {
    if (INVALID_OBJ_ID == mChaseTarget) {
        return BT::NodeStatus::FAIL;
    }

    auto owner = GetOwner();
    if (nullptr == owner or not IsPlayerInAttackRange()) {
        return BT::NodeStatus::FAIL;
    }

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus MonsterScript::Attack(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return BT::NodeStatus::FAIL;
    }

    if (INVALID_OBJ_ID == mChaseTarget) {
        return BT::NodeStatus::FAIL;
    }

    auto ownerRoom = owner->GetMyRoomIdx();
    auto chaseTarget = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetPlayer(mChaseTarget);
    if (nullptr == chaseTarget or false == chaseTarget->mSpec.active) {
        return BT::NodeStatus::FAIL;
    }

    auto currState = owner->mAnimationStateMachine.GetCurrState();
    if ((Packets::AnimationState_IDLE != currState and Packets::AnimationState_ATTACK != currState)
        and not owner->mAnimationStateMachine.IsChangable()) {
        return BT::NodeStatus::FAIL;
    }

    if (Packets::AnimationState_ATTACK == currState and owner->mAnimationStateMachine.IsChangable()) {
        return BT::NodeStatus::SUCCESS;
    }

    if (owner->mAnimationStateMachine.IsChangable()) {
        auto toTargetLook = MathUtil::Normalize(chaseTarget->GetPosition() - owner->GetPosition());
        owner->GetTransform()->SetLook(toTargetLook);
        owner->Attack();
    }

    return BT::NodeStatus::RUNNING;
}
