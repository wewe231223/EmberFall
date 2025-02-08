#pragma once

#include "Transform.h"

inline static constexpr float GRAVITY_FACTOR = 10.0f M_PER_SEC2;     // gravity (10.0 m/s^2)
inline static constexpr float DEFAULT_MESS = 10.0f;       // kg
inline static constexpr float DEFAULT_MAX_MOVE_SPEED = 20.0f M_PER_SEC; // m/s
inline static constexpr float DEFAULT_JUMP_FORCE = 3000.0f; // N = F = Mess * accel = kg * m / s^2

struct PhysicsFactor {
    float mess{ DEFAULT_MESS };
    float jumpForce{ mess * 300.f };
    float maxMoveSpeed{ DEFAULT_MAX_MOVE_SPEED };
};

// Physics Base
class Physics {
public:
    Physics();
    ~Physics();

public:
    void SetOnGround(bool state);
    void SetTransform(const std::shared_ptr<Transform>& transform);

    bool IsOnGround() const;
    bool IsMoving() const;
    bool IsMovingXZ() const;
    float GetMaxMoveSpeed() const;

    void SetMoveVelocity(const SimpleMath::Vector3& moveVel);
    void SetVelocity(const SimpleMath::Vector3& velocity);

    void Jump(const float deltaTime);

    void AddVelocity(const SimpleMath::Vector3& velocity);
    void Update(const float deltaTime);

private:
    void UpdateGravity(const float deltaTime);

public:
    PhysicsFactor mFactor{ };

private:
    bool mOnGround{ true };

    SimpleMath::Vector3 mMoveVelocity{ SimpleMath::Vector3::Zero };
    SimpleMath::Vector3 mVelocity{ SimpleMath::Vector3::Zero };
    std::weak_ptr<Transform> mTransform{ };
};
