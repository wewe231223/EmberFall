#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"
#include "BehaviorTreeBase.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner } { }

MonsterScript::~MonsterScript() { }

void MonsterScript::Init() {
    mMonsterBT.Build(std::static_pointer_cast<MonsterScript>(shared_from_this()));
}

void MonsterScript::Update(const float deltaTime) {
    // 02-22 움직임 잠시 비활성화
    //int rand = Random::GetRandom<INT32>(0, 1000);
    //if (0 == rand % 100) {
    //    mMoveDir = Random::GetRandomDirVec3();
    //    mMoveDir.y = 0.0f;
    //}

    //auto physics = GetOwner()->GetPhysics();
    //physics->Acceleration(mMoveDir, deltaTime);

    mMonsterBT.Update(deltaTime);
}

void MonsterScript::LateUpdate(const float deltaTime) { 
    if (mHp <= MathUtil::EPSILON) {
        GetOwner()->SetActive(false);
    }
}

void MonsterScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent) { }

void MonsterScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        mHp -= static_cast<AttackEvent*>(event)->damage;
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Monster [{}] Handling AttackEvent, HP: {}",
                                                        GetOwner()->GetId() - OBJECT_ID_START, mHp);
        break;

    default:
        break;
    }
}

BT::NodeStatus MonsterScript::SetRandomTargetLocation(const float deltaTime) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "[BehaviorTree Log]: Monster[{}]: Get New Random Target Location", GetOwner()->GetId() - OBJECT_ID_START);
    mTargetPos = Random::GetRandomVec3(SimpleMath::Vector3{ -10.0f, 0.0f, -10.0f }, SimpleMath::Vector3{ 10.0f, 0.0f, 10.0f });
    return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus MonsterScript::MoveTo(const float deltaTime) {
    auto owner = GetOwner();
    auto moveDir = mTargetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;
    if (SimpleMath::Vector3::DistanceSquared(mTargetPos, compareXZ) < 5.f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "[BehaviorTree Log]: Monster[{}]'s BT: MoveTo Success...", owner->GetId() - OBJECT_ID_START);
        return BT::NodeStatus::SUCCESS;
    }

    owner->GetPhysics()->Acceleration(moveDir, deltaTime);
    return BT::NodeStatus::RUNNING;
}
