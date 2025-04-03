#pragma once

#include "BehaviorTreeBase.h"

namespace BT {
    class BT_MonsterChase : public BehaviorTree {
    public:
        BT_MonsterChase() = default;
        virtual ~BT_MonsterChase() = default;

    public:
        virtual float CalculateDecideValue(const std::shared_ptr<Script>& ownerScript) const override;
        virtual void Build(const std::shared_ptr<Script>& ownerScript) override;
        virtual void DispatchGameEvent(GameEvent* event) override;
    };
}