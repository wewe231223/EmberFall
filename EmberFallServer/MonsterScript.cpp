#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"
#include "BehaviorTreeBase.h"

#include "ServerGameScene.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner, ObjectTag::MONSTER } {
    owner->SetEntityType(EntityType::MONSTER1);
    owner->RestoreHealth(10.0f);
    owner->GetPhysics()->mFactor.maxMoveSpeed = 1.3mps;
}

MonsterScript::~MonsterScript() { }

void MonsterScript::Init() {
    mMonsterBT.Build(std::static_pointer_cast<MonsterScript>(shared_from_this()));
}

void MonsterScript::Update(const float deltaTime) {
    mMonsterBT.Update(deltaTime);
}

void MonsterScript::LateUpdate(const float deltaTime) { 
    if (GetOwner()->HP() > MathUtil::EPSILON) {
        return;
    }

    auto currState = GetOwner()->mAnimationStateMachine.GetCurrState();
    if (AnimationState::DEAD == currState and GetOwner()->mAnimationStateMachine.IsChangable()) {

        PacketSC::PacketObjectDead packet{ sizeof(PacketSC::PacketObjectDead), PacketType::PACKET_OBJECT_DEAD };
        packet.objId = GetOwner()->GetId();

        gServerCore->SendAll(&packet);

        GetOwner()->SetActive(false);
        return;
    }

    if (AnimationState::DEAD != currState) {
        GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::DEAD);
    }
}

void MonsterScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void MonsterScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (AnimationState::DEAD == opponent->mAnimationStateMachine.GetCurrState() or AnimationState::DEAD == GetOwner()->mAnimationStateMachine.GetCurrState()) {
        return;
    }

    switch (opponent->GetTag()) {
    case ObjectTag::MONSTER:
        GetOwner()->GetPhysics()->SolvePenetration(impulse, opponent);
        break;

    case ObjectTag::CORRUPTED_GEM:
        GetOwner()->GetPhysics()->SolvePenetration(impulse, opponent);
        break;

    case ObjectTag::PLAYER:
        GetOwner()->GetPhysics()->SolvePenetration(impulse, opponent);
        break;

    default:
        break;
    }
}

void MonsterScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void MonsterScript::OnCollisionTerrain(const float height) { }

void MonsterScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        if (event->sender != event->receiver) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            GetOwner()->ReduceHealth(attackEvent->damage);
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Monster [{}] Attacked!!, HP: {}", GetOwner()->GetId(), GetOwner()->HP());
        }
        break;

    default:
        break;
    }
}

std::shared_ptr<GameObject> MonsterScript::GetChaseTarget() const {
    return mChaseTarget;
}

std::shared_ptr<GameObject>& MonsterScript::GetChaseTarget() {
    return mChaseTarget;
}

BT::NodeStatus MonsterScript::SetRandomTargetLocation(const float deltaTime) {
    GetOwner()->mAnimationStateMachine.ChangeState(AnimationState::IDLE);
    mTargetPos = Random::GetRandomVec3(SimpleMath::Vector3{ -100.0f, 0.0f, -100.0f }, SimpleMath::Vector3{ 100.0f, 0.0f, 100.0f });
    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus MonsterScript::MoveTo(const float deltaTime) {
    auto owner = GetOwner();
    owner->mAnimationStateMachine.ChangeState(AnimationState::MOVE_FORWARD);

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
    owner->GetPhysics()->Accelerate(moveDir);
    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MonsterScript::DetectPlayerInRange(const float deltaTime) {
    auto owner = GetOwner();

    auto gameWorld = owner->GetOwnGameScene();
    if (nullptr == gameWorld) {
        return BT::NodeStatus::FAIL;
    }

    decltype(auto) playerList = gameWorld->GetPlayers();
    for (const auto& player : playerList) {
        auto distance = SimpleMath::Vector3::Distance(player->GetPosition(), owner->GetPosition());
        if (distance < mPlayerDetectRange.Count()) {
            mChaseTarget = player;
            return BT::NodeStatus::SUCCESS;
        }
    }

    mChaseTarget = nullptr;
    return BT::NodeStatus::FAIL;
}

BT::NodeStatus MonsterScript::ChaseDetectedPlayer(const float deltaTime) {
    auto owner = GetOwner();
    owner->mAnimationStateMachine.ChangeState(AnimationState::MOVE_FORWARD);

    auto targetPos = mChaseTarget->GetPosition();
    auto moveDir = targetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    targetPos.y = 0.0f;
    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) < mAttackRange.Count() * mAttackRange.Count()) {
        return BT::NodeStatus::SUCCESS;
    }
    
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) > mPlayerDetectRange.Count() * mPlayerDetectRange.Count()) {
        return BT::NodeStatus::FAIL;
    }

    owner->GetTransform()->SetLook(moveDir);
    owner->GetPhysics()->Accelerate(moveDir);
    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MonsterScript::CheckPlayerInAttackRange(const float deltaTime) {
    if (nullptr == mChaseTarget) {
        return BT::NodeStatus::FAIL;
    }

    auto owner = GetOwner();

    auto targetPos = mChaseTarget->GetPosition();
    auto moveDir = targetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    targetPos.y = 0.0f;
    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;

    float attackRange = mAttackRange.Count();
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) > attackRange * attackRange) {
        return BT::NodeStatus::FAIL;
    }

    owner->mAnimationStateMachine.ChangeState(AnimationState::IDLE);
    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus MonsterScript::Attack(const float deltaTime) {
    if (nullptr == mChaseTarget) {
        return BT::NodeStatus::FAIL;
    }

    auto owner = GetOwner();

    auto currState = owner->mAnimationStateMachine.GetCurrState();
    if ((AnimationState::IDLE != currState and AnimationState::ATTACK != currState) 
        and not owner->mAnimationStateMachine.IsChangable()) {
        return BT::NodeStatus::FAIL;
    }

    if (AnimationState::ATTACK == currState and owner->mAnimationStateMachine.IsChangable()) {
        return BT::NodeStatus::SUCCESS;
    }

    if (owner->mAnimationStateMachine.IsChangable()) {
        auto toTargetLook = MathUtil::Normalize(mChaseTarget->GetPosition() - owner->GetPosition());
        owner->GetTransform()->SetLook(toTargetLook);
        owner->Attack();
        return BT::NodeStatus::RUNNING;
    }

    return BT::NodeStatus::RUNNING;
}
