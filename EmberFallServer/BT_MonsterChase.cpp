#include "pch.h"
#include "BT_MonsterChase.h"
#include "MonsterScript.h"

void BT::BT_MonsterChase::Build(const std::shared_ptr<Script>& ownerScript) {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    auto sequenceChase = std::make_unique<SequenceNode>();

    sequenceChase->AddChild<ActionNode>(std::bind_front(&MonsterScript::DetectPlayerInRange, owner.get()));
    sequenceChase->AddChild<ActionNode>(std::bind_front(&MonsterScript::ChaseDetectedPlayer, owner.get()));

    SetRoot(std::move(sequenceChase));
}
