#pragma once

#include "Script.h"

class MonsterScript : public Script {
public:
    MonsterScript(std::shared_ptr<class GameObject> owner);
    virtual ~MonsterScript();

public:
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;

    virtual void DispatchGameEvent(class GameEvent* event) override;

private:
    float mHp{ 100.0f };
    SimpleMath::Vector3 mMoveDir{ SimpleMath::Vector3::Zero };
};
