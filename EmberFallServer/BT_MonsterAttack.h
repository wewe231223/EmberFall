#pragma once

#include "BehaviorTreeBase.h"

namespace BT {
    class BT_MonsterAttack : public BehaviorTree {
    public:
        virtual void Build(const std::shared_ptr<Script>& ownerScript) override;
    };
}