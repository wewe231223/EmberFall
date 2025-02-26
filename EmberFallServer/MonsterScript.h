#pragma once

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

    virtual void OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

public:
    BT::NodeStatus SetRandomTargetLocation(const float deltaTime);
    BT::NodeStatus MoveTo(const float deltaTime);

private:
    float mHp{ 100.0f };
    SimpleMath::Vector3 mMoveDir{ SimpleMath::Vector3::Zero };
    SimpleMath::Vector3 mTargetPos{ SimpleMath::Vector3::Zero }; // TestTargetPos....

    BT::BehaviorTreeMonster mMonsterBT{ };
};
