#include "pch.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameRandom.h"
#include "Physics.h"
#include "BehaviorTreeBase.h"

#include "ServerGameScene.h"

MonsterScript::MonsterScript(std::shared_ptr<class GameObject> owner)
    : Script{ owner } { }

MonsterScript::~MonsterScript() { }

void MonsterScript::Init() {
    mMonsterBT.Build(std::static_pointer_cast<MonsterScript>(shared_from_this()));
}

void MonsterScript::Update(const float deltaTime) {
    mMonsterBT.Update(deltaTime);
}

void MonsterScript::LateUpdate(const float deltaTime) { 
    if (mHp <= MathUtil::EPSILON) {
        GetOwner()->SetActive(false);
    }
}

void MonsterScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void MonsterScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void MonsterScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

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
    if (SimpleMath::Vector3::DistanceSquared(mTargetPos, compareXZ) < 0.5f * 0.5f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "[BehaviorTree Log]: Monster[{}]'s BT: MoveTo Success...", owner->GetId() - OBJECT_ID_START);
        return BT::NodeStatus::SUCCESS;
    }

    owner->GetPhysics()->Acceleration(moveDir, deltaTime);
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
            gLogConsole->PushLog(DebugLevel::LEVEL_INFO, 
                "[BehaviorTree Log]: Monster[{}]'s BT: Start Chase Player[{}]!!!", owner->GetId(), player->GetId());
            mChaseTarget = player;
            return BT::NodeStatus::SUCCESS;
        }
    }

    mChaseTarget = nullptr;
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO,
        "[BehaviorTree Log]: Monster[{}]'s BT: Detect Player Failure", owner->GetId());
    return BT::NodeStatus::FAIL;
}

BT::NodeStatus MonsterScript::ChaseDetectedPlayer(const float deltaTime) {
    auto owner = GetOwner();
    auto targetPos = mChaseTarget->GetPosition();
    auto moveDir = targetPos - owner->GetPosition();
    moveDir.y = 0.0f;
    moveDir.Normalize();

    auto compareXZ = owner->GetPosition();
    compareXZ.y = 0.0f;
    if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) < 0.5f * 0.5f) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "[BehaviorTree Log]: Monster[{}]'s BT: Chase Success...", owner->GetId() - OBJECT_ID_START);
        return BT::NodeStatus::SUCCESS;
    }
    else if (SimpleMath::Vector3::DistanceSquared(targetPos, compareXZ) > mPlayerDetectRange.Count() * mPlayerDetectRange.Count()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "[BehaviorTree Log]: Monster[{}]'s BT: Chase Failure...", owner->GetId() - OBJECT_ID_START);
        return BT::NodeStatus::FAIL;
    }

    owner->GetPhysics()->Acceleration(moveDir, deltaTime);
    return BT::NodeStatus::RUNNING;
}
