#pragma once

#include "Transform.h"

// Physics Base
class Physics {
public:
    inline static constexpr float GRAVITY_FACTOR = 10.0f;     // gravity (10.0 m/s^2)
    inline static constexpr float DEFAULT_MESS = 10.0f;       // kg
    inline static constexpr float DEFAULT_MOVE_SPEED = 20.0f M_PER_SEC; // m/s
    inline static constexpr float DEFAULT_JUMP_SPEED = 100.0f M_PER_SEC;

public:
    Physics();
    ~Physics();

public:
    void SetTransform(const std::shared_ptr<Transform>& transform);
    void SetSpeed(const SimpleMath::Vector3& speed);
    void Jump();

    float GetMoveSpeed() const;

    void AddSpeed(const SimpleMath::Vector3& speed);

    void Update(const float deltaTime);

private:
    float mMoveSpeed{ DEFAULT_MOVE_SPEED };
    float mJumpSpeed{ DEFAULT_JUMP_SPEED };
    SimpleMath::Vector3 mSpeed{ };
    std::weak_ptr<Transform> mTransform{ };
};
