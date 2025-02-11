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
    mVelocity.y = (mFactor.jumpForce / mFactor.mass) * deltaTime; // v = F / mess * time (m/s)
}

void Physics::Acceleration(const SimpleMath::Vector3& dir, const float acceleration, const float deltaTime) {
    mVelocity += dir * acceleration * deltaTime;

    ClampVelocity();
}

void Physics::Acceleration(const SimpleMath::Vector3& dir, const float deltaTime) {
    mVelocity += dir * mFactor.acceleration * deltaTime;

    ClampVelocity();
}

void Physics::AddVelocity(const SimpleMath::Vector3& speed, const float deltaTime) {
    mVelocity += speed;

    ClampVelocity();
}

void Physics::AddForce(const SimpleMath::Vector3& force, const float deltaTime) {
    mVelocity += (force / mFactor.mass) * deltaTime;
    
    ClampVelocity();
}

void Physics::Update(const float deltaTime) {
    if (mTransform.expired()) {
        return;
    }

    float speed = mVelocity.Length();
    SimpleMath::Vector3 moveDir = mVelocity;
    moveDir.Normalize();

    UpdateFriction(deltaTime, moveDir, speed);
    UpdateGravity(deltaTime, moveDir, speed);   // 중력 적용

    auto transform = mTransform.lock();
    transform->Move(mVelocity * deltaTime);
}

void Physics::ClampVelocity() {
    SimpleMath::Vector3 velocityXZ = mVelocity;
    velocityXZ.y = 0.0f;

    if (velocityXZ.LengthSquared() > mFactor.maxMoveSpeed * mFactor.maxMoveSpeed) {
        velocityXZ.Normalize();
        velocityXZ = velocityXZ * mFactor.maxMoveSpeed;
        mVelocity.x = velocityXZ.x;
        mVelocity.z = velocityXZ.z;
    }
}

void Physics::UpdateFriction(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed) {
    if (false == IsOnGround()) {
        return;
    }

    float normalForce = mFactor.mass * GRAVITY_ACCELERATION;
    SimpleMath::Vector3 frictionForce = -moveDir * mFactor.friction * normalForce;
    frictionForce.y = 0.0f; // Y축 계산 X
    SimpleMath::Vector3 frictionAcc = frictionForce / mFactor.mass;
    SimpleMath::Vector3 resultVelocity = mVelocity + frictionAcc * deltaTime;

    mVelocity.x = (mVelocity.x * resultVelocity.x < 0.0f) ? 0.0f : resultVelocity.x;
    mVelocity.z = (mVelocity.z * resultVelocity.z < 0.0f) ? 0.0f : resultVelocity.z;
}

void Physics::UpdateGravity(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed) {
    if (IsOnGround()) {
        mVelocity.y = 0.0f;
        return;
    }

    SimpleMath::Vector3 dragForce = SimpleMath::Vector3::Up * mFactor.dragCoeffi * speed * speed;
    SimpleMath::Vector3 dragAcceleration = dragForce / mFactor.mass;

    // 최종 가속도 = 중력 + 공기 저항
    SimpleMath::Vector3 acceleration = SimpleMath::Vector3::Down * GRAVITY_ACCELERATION + dragAcceleration;
    mVelocity += acceleration * deltaTime;
}
