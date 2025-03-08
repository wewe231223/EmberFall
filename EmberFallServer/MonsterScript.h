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
#include "BehaviorTreeMonster.h"

class MonsterScript : public Script {
public:
    MonsterScript(std::shared_ptr<class GameObject> owner);
    virtual ~MonsterScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

public:
    // 행동트리 관련 함수들 return NodeStatus
    BT::NodeStatus SetRandomTargetLocation(const float deltaTime);
    BT::NodeStatus MoveTo(const float deltaTime);

    BT::NodeStatus DetectPlayerInRange(const float deltaTime);
    BT::NodeStatus ChaseDetectedPlayer(const float deltaTime);

private:
    float mHp{ 100.0f };
    SimpleMath::Vector3 mMoveDir{ SimpleMath::Vector3::Zero };
    SimpleMath::Vector3 mTargetPos{ SimpleMath::Vector3::Zero }; // TestTargetPos....

    // range to detecting player 
    std::shared_ptr<class GameObject> mChaseTarget{ nullptr };
    GameUnits::GameUnit<GameUnits::Meter> mPlayerDetectRange{ 10.0m };

    BT::BehaviorTreeMonster mMonsterBT{ };
};
