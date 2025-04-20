#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"
#include "BehaviorTreeBase.h"
#include "Sector.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner, ObjectTag::MONSTER, ScriptType::MONSTER } {
    owner->mSpec.entity = Packets::EntityType_MONSTER;
    owner->mSpec.hp = 100.0f;
    owner->GetPhysics()->mFactor.maxMoveSpeed = 3.3mps;
}

MonsterScript::~MonsterScript() { }

bool MonsterScript::IsDead() const {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return true;
    }

    return Packets::AnimationState_DEAD == owner->mAnimationStateMachine.GetCurrState();
}

void MonsterScript::Init() {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.attackable = true;
    spec.hp = 100.0f;
    mMonsterBT.Build(std::static_pointer_cast<MonsterScript>(shared_from_this()));
}

void MonsterScript::Update(const float deltaTime) {
    mMonsterBT.Update(deltaTime);

    //gSectorSystem->UpdateEntityMove(GetOwner());
}

void MonsterScript::LateUpdate(const float deltaTime) { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (owner->mSpec.hp > MathUtil::EPSILON) {
        return;
    }

    auto currState = owner->mAnimationStateMachine.GetCurrState();
    if (Packets::AnimationState_DEAD == currState and owner->mAnimationStateMachine.IsChangable()) {
        owner->mSpec.active = false;
        return;
    }

    if (Packets::AnimationState_DEAD != currState) {
        owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_DEAD);
    }
}

void MonsterScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    owner->GetPhysics()->SolvePenetration(impulse);
}

void MonsterScript::OnCollisionTerrain(const float height) { }

void MonsterScript::DispatchGameEvent(GameEvent* event) { 
    if (IsDead()) {
        return;
    }

    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        if (event->sender != event->receiver) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            owner->mSpec.hp -= attackEvent->damage;
            
            owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACKED);

            mMonsterBT.Interrupt();
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Monster [{}] Attacked!!, HP: {}", owner->GetId(), owner->mSpec.hp);
        }
        break;

    default:
        break;
    }

    mMonsterBT.DispatchGameEvent(event);
}

std::shared_ptr<GameObject> MonsterScript::GetChaseTarget() const {
    return mChaseTarget;
}

std::shared_ptr<GameObject>& MonsterScript::GetChaseTarget() {
    return mChaseTarget;
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

    //auto gameWorld = owner->GetOwnGameScene();
    //if (nullptr == gameWorld) {
    //    return BT::NodeStatus::FAIL;
    //}

    //decltype(auto) playerList = gameWorld->GetPlayers();
    //for (const auto& player : playerList) {
    //    auto distance = SimpleMath::Vector3::Distance(player->GetPosition(), owner->GetPosition());
    //    if (distance < mPlayerDetectRange.Count()) {
    //        mChaseTarget = player;
    //        return BT::NodeStatus::SUCCESS;
    //    }
    //}

    mChaseTarget = nullptr;
    return BT::NodeStatus::FAIL;
}

BT::NodeStatus MonsterScript::ChaseDetectedPlayer(const float deltaTime) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return BT::NodeStatus::FAIL;
    }
    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_MOVE_FORWARD);

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
    owner->GetPhysics()->Accelerate(moveDir, GetOwner()->GetDeltaTime());
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

    owner->mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE);
    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus MonsterScript::Attack(const float deltaTime) {
    if (nullptr == mChaseTarget) {
        return BT::NodeStatus::FAIL;
    }

    auto owner = GetOwner();

    auto currState = owner->mAnimationStateMachine.GetCurrState();
    if ((Packets::AnimationState_IDLE != currState and Packets::AnimationState_ATTACK != currState)
        and not owner->mAnimationStateMachine.IsChangable()) {
        return BT::NodeStatus::FAIL;
    }

    if (Packets::AnimationState_ATTACK == currState and owner->mAnimationStateMachine.IsChangable()) {
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
