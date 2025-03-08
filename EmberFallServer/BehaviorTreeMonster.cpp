#include "pch.h"
#include "BehaviorTreeMonster.h"
#include "MonsterScript.h"

#include "BT_MonsterChase.h"

void BT::BehaviorTreeMonster::Build(const std::shared_ptr<Script>& ownerScript) {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);

    BT_MonsterChase chase;
    chase.Build(ownerScript);

    auto sequenceMoveRandomLoc = std::make_unique<SequenceNode>();
    sequenceMoveRandomLoc->AddChild<ActionNode>(std::bind_front(&MonsterScript::SetRandomTargetLocation, owner.get()));
    sequenceMoveRandomLoc->AddChild<ActionNode>(std::bind_front(&MonsterScript::MoveTo, owner.get()));

    auto selectorNode = std::make_unique<SelectorNode>();
    SetRoot(std::move(selectorNode));

    SetOtherTree(chase);
    SetChild(std::move(sequenceMoveRandomLoc));
}
