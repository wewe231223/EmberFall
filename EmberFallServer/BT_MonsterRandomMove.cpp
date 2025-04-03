#include "pch.h"
#include "BT_MonsterRandomMove.h"
#include "MonsterScript.h"

float BT::BT_MonsterRandomMove::CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const {
    return 0.3f;
}

void BT::BT_MonsterRandomMove::Build(const std::shared_ptr<Script>& ownerScript) {
    auto owner = std::static_pointer_cast<MonsterScript>(ownerScript);

    auto sequenceMoveRandomLoc = std::make_unique<SequenceNode>();
    sequenceMoveRandomLoc->AddChild<ActionNode>(std::bind_front(&MonsterScript::SetRandomTargetLocation, owner.get()));
    sequenceMoveRandomLoc->AddChild<ActionNode>(std::bind_front(&MonsterScript::MoveTo, owner.get()));

    SetRoot(std::move(sequenceMoveRandomLoc));
}

void BT::BT_MonsterRandomMove::DispatchGameEvent(GameEvent* event) { }
