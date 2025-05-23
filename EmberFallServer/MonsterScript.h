#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MonsterScript.h
// 
// 2025 - 02 - 10 : MonsterScript
// 
//        02 - 27 : 플레이어를 쫓아가는 행동을 구현하려면...
//                  결국 게임 월드에 존재하는 게임 오브젝트 혹은 플레이어 리스트에 접근해야함.
//                  
//                  일단 게임 씬을 GameObject가 참조하게 만들자...
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

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse);
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

public:
    NetworkObjectIdType GetChaseTarget() const;

    bool IsPlayerInAttackRange() const;

    // 행동트리 관련 함수들 return NodeStatus
    BT::NodeStatus SetRandomTargetLocation(const float deltaTime);
    BT::NodeStatus MoveTo(const float deltaTime);

    BT::NodeStatus DetectPlayerInRange(const float deltaTime);
    BT::NodeStatus ChaseDetectedPlayer(const float deltaTime);

    BT::NodeStatus CheckPlayerInAttackRange(const float deltaTime);
    BT::NodeStatus Attack(const float deltaTime);

private:
    SimpleMath::Vector3 mMoveDir{ SimpleMath::Vector3::Zero };
    SimpleMath::Vector3 mTargetPos{ SimpleMath::Vector3::Zero }; // TestTargetPos....

    // range to detecting player 
    NetworkObjectIdType mChaseTarget{ INVALID_OBJ_ID };
    GameUnits::GameUnit<GameUnits::Meter> mAttackRange{ 1.0m };
    GameUnits::GameUnit<GameUnits::Meter> mPlayerDetectRange{ 10.0m };

    BT_Monster mMonsterBT{ };
};
