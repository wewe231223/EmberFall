#include "pch.h"
#include "Physics.h"

Physics::Physics() { }

Physics::~Physics() { }

bool Physics::IsMoving() const {
    return false == MathUtil::IsVectorZero(mVelocity);
}

bool Physics::IsMovingXZ() const {
    SimpleMath::Vector3 xzVelocity{ mVelocity.x, 0.0f, mVelocity.z };
    return false == MathUtil::IsVectorZero(mVelocity);
}

bool Physics::IsOnGround() const {
    return mOnGround;
}

void Physics::SetOnGround(bool state) {
    mOnGround = state;
}

void Physics::SetTransform(const std::shared_ptr<Transform>& transform) {
    mTransform = transform;
}

void Physics::Jump(const float deltaTime) {
    if (false == IsOnGround()) {
        return;
    }

    mOnGround = false;
    mJumpForce = mFactor.jumpForce;
    mVelocity.y = mFactor.jumpForce / mFactor.mess * deltaTime; // v = F / mess * time (m/s)
}

void Physics::AddVelocity(const SimpleMath::Vector3& speed) {
    mVelocity += speed;
}

void Physics::Update(const float deltaTime) {
    if (mTransform.expired()) {
        return;
    }

    auto transform = mTransform.lock();
    UpdateGravity(deltaTime);   // 중력 적용

    transform->Move(mVelocity * deltaTime);

    ResetVelocity();
}

void Physics::ResetVelocity() {
    mVelocity = SimpleMath::Vector3::Zero;
}

void Physics::UpdateGravity(const float deltaTime) {
    if (IsOnGround()) {
        mJumpForce = 0.0f;
    }

    mJumpForce -= mFactor.mess * GRAVITY_FACTOR; // kg * m / s^2
    mVelocity.y += mJumpForce / mFactor.mess * deltaTime;
}
