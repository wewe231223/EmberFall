#pragma once

#include "BuffScript.h"

class BuffHealScript : public BuffScript {
public:
    BuffHealScript(std::shared_ptr<class GameObject> owner, float delay=0.5f, float duration=5.0f, float healpoint=5.0f);
    virtual ~BuffHealScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    float mDelay{ };
    float mHealPoint{ };
    SysClock::time_point mDelayCounter{ };
};

