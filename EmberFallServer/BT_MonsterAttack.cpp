#include "pch.h"
#include "BT_MonsterAttack.h"
#include "MonsterScript.h"

float BT::BT_MonsterAttack::CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    if (BT::NodeStatus::SUCCESS == owner->CheckPlayerInAttackRange(0.0f, 3.0f)) {
        return 1.0f;
    }

    return 0.0f;
}

void BT::BT_MonsterAttack::Build(const std::shared_ptr<Script>& ownerScript) {
    static float attackRange{ 3.0f };
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    auto sequenceAttack = std::make_unique<SequenceNode>();

    sequenceAttack->AddChild<ActionNode>(std::bind_front(&MonsterScript::CheckPlayerInAttackRange, owner.get(), attackRange));
    sequenceAttack->AddChild<ActionNode>(std::bind_front(&MonsterScript::Attack, owner.get()));

    SetRoot(std::move(sequenceAttack));
}

void BT::BT_MonsterAttack::DispatchGameEvent(GameEvent* event) { }

