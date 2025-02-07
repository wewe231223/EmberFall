#include "pch.h"
#include "Physics.h"

Physics::Physics() { }

Physics::~Physics() { }

void Physics::SetTransform(const std::shared_ptr<Transform>& transform) { 
    mTransform = transform;
}

void Physics::SetSpeed(const SimpleMath::Vector3& speed) {
    mSpeed = speed;
}

void Physics::Jump() {
    mSpeed.y = mJumpSpeed;
}

float Physics::GetMoveSpeed() const {
    return mMoveSpeed;
}

void Physics::AddSpeed(const SimpleMath::Vector3& speed) {
    mSpeed += speed;
}

void Physics::Update(const float deltaTime) {
    if (mTransform.expired()) {
        return;
    }

    auto gravity = SimpleMath::Vector3{ 0.0f, -DEFAULT_MESS * GRAVITY_FACTOR * deltaTime, 0.0f };
    mSpeed += gravity;
    mTransform.lock()->Move(mSpeed * deltaTime);
}
