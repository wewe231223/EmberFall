#include "pch.h"
#include "BehaviorTreeMonster.h"
#include "MonsterScript.h"

void BT::BehaviorTreeMonster::Build(const std::shared_ptr<Script>& ownerScript) {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    auto sequenceNode = std::make_unique<SequenceNode>();

    sequenceNode->AddChild<ActionNode>(std::bind_front(&MonsterScript::SetRandomTargetLocation, owner.get()));
    sequenceNode->AddChild<ActionNode>(std::bind_front(&MonsterScript::MoveTo, owner.get()));

    SetRoot(std::move(sequenceNode));
}
