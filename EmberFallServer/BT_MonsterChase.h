#pragma once

#include "BehaviorTreeBase.h"

namespace BT {
    class BT_MonsterChase : public BehaviorTree {
    public:
        virtual void Build(const std::shared_ptr<Script>& ownerScript) override;
    };
}