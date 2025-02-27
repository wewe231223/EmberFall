#include "pch.h"
#include "BehaviorTreeMonster.h"
#include "MonsterScript.h"

void BT::BehaviorTreeMonster::Build(const std::shared_ptr<Script>& ownerScript) {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);
    auto sequenceChase = std::make_unique<SequenceNode>();

    sequenceChase->AddChild<ActionNode>(std::bind_front(&MonsterScript::DetectPlayerInRange, owner.get()));
    sequenceChase->AddChild<ActionNode>(std::bind_front(&MonsterScript::ChaseDetectedPlayer, owner.get()));

    auto sequenceMoveRandomLoc = std::make_unique<SequenceNode>();
    sequenceMoveRandomLoc->AddChild<ActionNode>(std::bind_front(&MonsterScript::SetRandomTargetLocation, owner.get()));
    sequenceMoveRandomLoc->AddChild<ActionNode>(std::bind_front(&MonsterScript::MoveTo, owner.get()));

    auto selectorNode = std::make_unique<SelectorNode>();
    selectorNode->AddChild(std::move(sequenceChase));
    selectorNode->AddChild(std::move(sequenceMoveRandomLoc));

    SetRoot(std::move(selectorNode));
}
