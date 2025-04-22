#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Physics.h
// 
// 2025 - 02 - 10   : 물리연산을 담당할 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Transform.h"
inline static auto GRAVITY_ACCELERATION = 1.0G;
inline static auto DEFAULT_ACCELERATION = 20.0mps2;
inline static auto DEFAULT_MASS = 70.0kg;       // kg
inline static auto DEFAULT_MAX_MOVE_SPEED = 5.0mps; // m/s
inline static auto DEFAULT_JUMP_FORCE = 6000.0N; // N = F = Mess * accel = kg * m / s^2
inline static auto DEFAULT_JUMP_TIME = 0.2sec;

struct PhysicsFactor {
    GameUnits::GameUnit<GameUnits::StandardAccel> acceleration{ DEFAULT_ACCELERATION };
    GameUnits::GameUnit<GameUnits::StandardForce> jumpForce{ DEFAULT_JUMP_FORCE };
    GameUnits::GameUnit<GameUnits::StandardSpeed> maxMoveSpeed{ DEFAULT_MAX_MOVE_SPEED };
    GameUnits::GameUnit<GameUnits::StandardTime> jumpTime{ DEFAULT_JUMP_TIME };
    GameUnits::GameUnit<GameUnits::StandardMass> mass{ DEFAULT_MASS };

    float dragCoeffi{ 0.1f };
    float friction{ 1.0f };
};

// Physics Base
class Physics {
public:
    Physics();
    ~Physics();

public:
    void SetOnGround(bool state);
    void SetOnOtherObject(bool state);
    void SetTransform(const std::shared_ptr<Transform>& transform);

    bool IsOnGround() const;
    bool IsOnOtherObject() const;
    bool IsMoving() const;
    bool IsMovingXZ() const;

    float GetSpeed() const;
    SimpleMath::Vector3 GetMoveDir() const;

    void Disable();
    void Reset();

    void CheckAndJump(const float deltaTime);

    void ResizeVelocity(float speed);

    void Accelerate(const SimpleMath::Vector3& dir);
    void Accelerate(const SimpleMath::Vector3& dir, const float acceleration);

    void AddVelocity(const SimpleMath::Vector3& velocity);
    void AddForce(const SimpleMath::Vector3& force);
    void AddForce(const SimpleMath::Vector3& dir, const float force);
    void Update(const float deltaTime);
    void LateUpdate(const float deltaTime);

    void SolvePenetration(const SimpleMath::Vector3& penetrationVec, const std::shared_ptr<class GameObject>& opponent);

private:
    void ClampVelocity();
    void UpdateFriction(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed);
    void UpdateGravity(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed);

public:
    PhysicsFactor mFactor{ };

private:
    bool mActive{ true };
    bool mOnGround{ true };
    bool mOnOtherObject{ true };

    SimpleMath::Vector3 mPrevImpulse{ };
    SimpleMath::Vector3 mVelocity{ SimpleMath::Vector3::Zero };
    std::weak_ptr<Transform> mTransform{ };
};
