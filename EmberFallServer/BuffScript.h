#pragma once

#include "Script.h"

using Buff = uint8_t;

enum class BuffType : uint8_t {
    NONE                = 0b0000'0000,
    BUFF_HEAL           = 0b0000'0001,
    BUFF_GAURD          = 0b0000'0010,
    DEBUFF_DISARM       = 0b0000'0100,
    DEBUFF_BLIND        = 0b0000'1000
};

class BuffScript : public Script {
public:
    BuffScript(std::shared_ptr<GameObject> owner, float duration);
    virtual ~BuffScript();

public:
    Buff GetBuffType() const;

    virtual void Init() abstract;

    virtual void Update(const float deltaTime) abstract;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    virtual void OnCollisionTerrain(const float height) abstract;

    virtual void DispatchGameEvent(struct GameEvent* event) abstract;

private:
    bool mActive{ true };
    float mDuration{ };
    float mDurationCounter{ };
    Buff mBuff{ static_cast<Buff>(BuffType::NONE) };
};
