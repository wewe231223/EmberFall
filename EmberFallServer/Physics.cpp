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

void Physics::SetMoveVelocity(const SimpleMath::Vector3& moveVel) {
    mMoveVelocity = moveVel;
}

void Physics::SetVelocity(const SimpleMath::Vector3& velocity) {
    mVelocity = velocity;
}

void Physics::Jump(const float deltaTime, const float jumpForce) {
    if (false == IsOnGround()) {
        return;
    }

    mVelocity.y = jumpForce / DEFAULT_MESS * deltaTime; // v = F / mess * time (m/s)
}

float Physics::GetMaxMoveSpeed() const {
    return mMoveSpeedMax;
}

void Physics::AddVelocity(const SimpleMath::Vector3& speed) {
    mVelocity += speed;
}

void Physics::Update(const float deltaTime) {
    if (mTransform.expired()) {
        return;
    }

    UpdateGravity(deltaTime);   // 중력 적용
    // 입력으로 인해 생기는 이동속도 추가
    mTransform.lock()->Move(mMoveVelocity * deltaTime);
    mTransform.lock()->Move(mVelocity * deltaTime);
}

void Physics::UpdateGravity(const float deltaTime) {
    if (IsOnGround()) {
        mVelocity.y = 0.0f;
        return;
    }

    auto speed = GRAVITY_FACTOR * deltaTime; // m/s
    mVelocity.y -= speed;
}
