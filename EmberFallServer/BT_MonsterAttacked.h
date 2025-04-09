#pragma once

#include "BehaviorTreeBase.h"

namespace BT {
    class BT_MonsterAttacked : public BehaviorTree {
    public:
        BT_MonsterAttacked() = default;
        virtual ~BT_MonsterAttacked() = default;

    public:
        virtual float CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const override;
        virtual void Build(const std::shared_ptr<Script>& ownerScript) override;
        virtual void DispatchGameEvent(GameEvent* event) override;
    };
}
