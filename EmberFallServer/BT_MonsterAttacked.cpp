#include "pch.h"
#include "BT_MonsterAttacked.h"
#include "GameObject.h"

float BT::BT_MonsterAttacked::CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const {
    auto owner = ownerScript->GetOwner();
    if (nullptr == owner) {
        return 0.0f;
    }

    auto state = owner->mAnimationStateMachine.GetCurrState();
    if (Packets::AnimationState_ATTACKED == state) {
        return 1.5f;
    }

    return 0.0f;
}

void BT::BT_MonsterAttacked::Build(const std::shared_ptr<Script>& ownerScript) {
    static auto waitingFn = [=](std::shared_ptr<Script>& ownerScript, const float deltaTime) {
        auto owner = ownerScript->GetOwner();
        if (nullptr == owner) {
            return NodeStatus::FAIL;
        }

        if (owner->mAnimationStateMachine.GetRemainDuration() <= 0.0f) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Waiting Attacked Animation End");
            return NodeStatus::SUCCESS;
        }

        return NodeStatus::RUNNING;
    };

    SetRoot(std::make_unique<ActionNode>(std::bind_front(waitingFn, ownerScript)));
}

void BT::BT_MonsterAttacked::DispatchGameEvent(GameEvent* event) { }
