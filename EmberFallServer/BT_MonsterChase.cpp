#include "pch.h"
#include "BT_MonsterChase.h"
#include "MonsterScript.h"
#include "GameObject.h"
#include "GameTimer.h"

float BT::BT_MonsterChase::CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    if (nullptr == owner) {
        return 0.0f;
    }

    auto ownerObj = owner->GetOwner();
    if (nullptr == ownerObj) {
        return 0.0f;
    }

    if (ownerObj->mAnimationStateMachine.IsChangable() and BT::NodeStatus::SUCCESS == owner->DetectPlayerInRange(1.0f)) {
        return 0.8f;
    }

    return 0.0f;
}

void BT::BT_MonsterChase::Build(const std::shared_ptr<Script>& ownerScript) {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    auto sequenceChase = std::make_unique<SequenceNode>();

    sequenceChase->AddChild<ConditionNode>(std::bind_front(&MonsterScript::DetectPlayerInRange, owner.get()));
    sequenceChase->AddChild<ActionNode>(std::bind_front(&MonsterScript::ChaseDetectedPlayer, owner.get()));

    SetRoot(std::move(sequenceChase));
}

void BT::BT_MonsterChase::DispatchGameEvent(GameEvent* event) { }
