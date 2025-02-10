#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Input.h
// 
// 2025 - 02 - 10   : 물리연산을 담당할 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Transform.h"

inline static constexpr float GRAVITY_ACCELERATION = 10.0f M_PER_SEC2;     // gravity (10.0 m/s^2)
inline static constexpr float DEFAULT_ACCELERATION = 10.0f M_PER_SEC2;
inline static constexpr float DEFAULT_MASS = 70.0f;       // kg
inline static constexpr float DEFAULT_MAX_MOVE_SPEED = 20.0f M_PER_SEC; // m/s
inline static constexpr float DEFAULT_JUMP_FORCE = 10000.0f; // N = F = Mess * accel = kg * m / s^2
inline static constexpr float DEFAULT_JUMP_TIEM = 0.2f;

struct PhysicsFactor {
    float acceleration{ DEFAULT_ACCELERATION };
    float mass{ DEFAULT_MASS };
    float jumpForce{ mass * 300.f };
    float dragCoeffi{ 0.5f };
    float maxMoveSpeed{ DEFAULT_MAX_MOVE_SPEED };
    float friction{ 1.2f };
    float jumpTime{ 0.2f };
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

    void Jump(const float deltaTime);

    void Acceleration(const SimpleMath::Vector3& dir, const float acceleration, const float deltaTime);
    void Acceleration(const SimpleMath::Vector3& dir, const float deltaTime);
    void AddVelocity(const SimpleMath::Vector3& velocity, const float deltaTime);
    void AddForce(const SimpleMath::Vector3& force, const float deltaTime);
    virtual void Update(const float deltaTime);

private:
    void ClampVelocity();
    void UpdateFriction(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed);
    void UpdateGravity(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed);

public:
    PhysicsFactor mFactor{ };

private:
    bool mOnGround{ true };

    SimpleMath::Vector3 mVelocity{ SimpleMath::Vector3::Zero };
    std::weak_ptr<Transform> mTransform{ };
};
