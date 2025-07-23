#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MonsterScript.h
// 
// 2025 - 02 - 10 : MonsterScript
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Script.h"
#include "BT_Monster.h"

class MonsterScript : public Script {
public:
    MonsterScript(std::shared_ptr<class GameObject> owner);
    virtual ~MonsterScript();

public:
    virtual void Init() override;

    virtual void Update(const float time) override;
    virtual void LateUpdate(const float time) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

public:
    NetworkObjectIdType GetChaseTarget() const;

    bool IsPlayerInAttackRange() const;

    // 행동트리 관련 함수들 return NodeStatus
    BT::NodeStatus SetRandomTargetLocation(const float time);
    BT::NodeStatus MoveTo(const float time);
    BT::NodeStatus Wait(const float time);

    BT::NodeStatus DetectPlayerInRange(const float time);
    BT::NodeStatus ChaseDetectedPlayer(const float time);

    BT::NodeStatus CheckPlayerInAttackRange(const float time);
    BT::NodeStatus Attack(const float time);

private:
    float mWaitTime{ };
    float mWaitTimeCounter{ };

    SimpleMath::Vector3 mMoveDir{ SimpleMath::Vector3::Zero };
    SimpleMath::Vector3 mTargetPos{ SimpleMath::Vector3::Zero };

    NetworkObjectIdType mChaseTarget{ INVALID_OBJ_ID };
    GameUnits::GameUnit<GameUnits::Meter> mAttackRange{ 1.0m };
    GameUnits::GameUnit<GameUnits::Meter> mPlayerDetectRange{ 10.0m };

    BT_Monster mMonsterBT{ };
};
