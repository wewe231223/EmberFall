#include "pch.h"
#include "Physics.h"
#include "GameObject.h"
#include "GameTimer.h"

Physics::Physics() { }

Physics::~Physics() { }

bool Physics::IsMoving() const {
    return false == MathUtil::IsVectorZero(mVelocity);
}

bool Physics::IsMovingXZ() const {
    SimpleMath::Vector3 xzVelocity{ mVelocity.x, 0.0f, mVelocity.z };
    return false == MathUtil::IsVectorZero(mVelocity);
}

void Physics::Disable() {
    mActive = false;
}

void Physics::Reset() {
    mActive = true;
    mOnGround = false;
    mOnOtherObject = false;

    mPrevImpulse = SimpleMath::Vector3::Zero;
    mVelocity = SimpleMath::Vector3::Zero;
}

bool Physics::IsOnGround() const {
    return mOnGround;
}

bool Physics::IsOnOtherObject() const {
    return mOnOtherObject;
}

void Physics::SetOnGround(bool state) {
    mOnGround = state;

    if (true == state) {
        mVelocity.y = 0.0f;
    }
}

void Physics::SetOnOtherObject(bool state) {
    mOnOtherObject = state;

    if (true == state) {
        mVelocity.y = 0.0f;
    }
}

void Physics::SetTransform(const std::shared_ptr<Transform>& transform) {
    mTransform = transform;
}

void Physics::CheckAndJump(const float deltaTime) {
    if (false == IsOnGround() and false == IsOnOtherObject()) {
        return;
    }

    mOnGround = false;
    mOnOtherObject = false;
    
    // v = F / mess * time (m/s)
    auto accel = mFactor.jumpForce / mFactor.mass;
    mVelocity.y = (accel * GameUnits::ToUnit<GameUnits::StandardTime>(deltaTime)).Count(); 
    
    mTransform.lock()->Translate(SimpleMath::Vector3{ 0.0f, 1.0f, 0.0f });
}

void Physics::ResizeVelocity(float speed) {
    SimpleMath::Vector3 velocityXZ = mVelocity;
    velocityXZ.y = 0.0f;

    velocityXZ.Normalize();
    velocityXZ *= speed;

    mVelocity.x = velocityXZ.x;
    mVelocity.z = velocityXZ.z;
}

void Physics::Accelerate(const SimpleMath::Vector3& dir, const float acceleration) {
    mVelocity += dir * acceleration * StaticTimer::GetDeltaTime();

    ClampVelocity();
}

void Physics::Accelerate(const SimpleMath::Vector3& dir) {
    auto speed = mFactor.acceleration * GameUnits::ToUnit<GameUnits::StandardTime>(StaticTimer::GetDeltaTime());
    mVelocity += GameUnits::ToVelocity(dir, speed);

    ClampVelocity();
}

void Physics::AddVelocity(const SimpleMath::Vector3& speed) {
    mVelocity += speed;

    ClampVelocity();
}

void Physics::AddForce(const SimpleMath::Vector3& force) {
    mVelocity += (force / mFactor.mass.Count()) * StaticTimer::GetDeltaTime();
    
    ClampVelocity();
}

void Physics::AddForce(const SimpleMath::Vector3& dir, const float force) {
    mVelocity += ((dir * force) / mFactor.mass.Count()) * StaticTimer::GetDeltaTime();

    ClampVelocity();
}

void Physics::Update(const float deltaTime) {
    if (mTransform.expired() or false == mActive) {
        return;
    }

    float speed = mVelocity.Length();
    SimpleMath::Vector3 moveDir = mVelocity;
    moveDir.Normalize();
    UpdateGravity(deltaTime, moveDir, speed);   // 중력 적용
    UpdateFriction(deltaTime, moveDir, speed);

    auto transform = mTransform.lock();
    transform->Translate(mVelocity * deltaTime);
}

void Physics::LateUpdate(const float deltaTime) { }

void Physics::SolvePenetration(const SimpleMath::Vector3& penetrationVec, const std::shared_ptr<GameObject>& opponent) {
    auto transform = mTransform.lock();

    float myMass = mFactor.mass.Count();
    float opponentMass = opponent->GetPhysics()->mFactor.mass.Count();
    // 내가 무거울 수록 덜 밀려나는 구조.
    float coefficient = opponentMass / (myMass + opponentMass); // 0.0f ~ 1.0f 사이 값.
    auto repulsiveVec = coefficient * penetrationVec;
    transform->Translate(repulsiveVec);

    bool onOtherObject{ false };
    bool amIOnGround = IsOnGround() or IsOnOtherObject();
    bool isOpponentOnGround = opponent->GetPhysics()->IsOnGround() or opponent->GetPhysics()->IsOnOtherObject();

    //if (not amIOnGround and (transform->GetPrevPosition().y > (obb2.Center.y + obb2.Extents.y))) {
    //    onOtherObject = true;
    //    transform->SetY(obb1.Extents.y + obb2.Center.y + obb2.Extents.y);
    //}

    SetOnOtherObject(onOtherObject);
}

void Physics::ClampVelocity() {
    SimpleMath::Vector3 velocityXZ = mVelocity;
    velocityXZ.y = 0.0f;

    if (velocityXZ.LengthSquared() > (mFactor.maxMoveSpeed.Count() * mFactor.maxMoveSpeed.Count())) {
        velocityXZ.Normalize();
        velocityXZ = velocityXZ * mFactor.maxMoveSpeed.Count();
        mVelocity.x = velocityXZ.x;
        mVelocity.z = velocityXZ.z;
    }
}

void Physics::UpdateFriction(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed) {
    if (MathUtil::IsEqualVector(MathUtil::AbsVector(moveDir), SimpleMath::Vector3::Up)) {
        return;
    }

    auto inverseDir = -moveDir;
    auto normalForce = mFactor.mass * GRAVITY_ACCELERATION;
    SimpleMath::Vector3 frictionForce = inverseDir * normalForce.Count() * mFactor.friction;

    frictionForce.y = 0.0f; // Y축 계산 X
    SimpleMath::Vector3 frictionAcc = frictionForce / mFactor.mass.Count();
    SimpleMath::Vector3 resultVelocity = mVelocity + frictionAcc * deltaTime;

    mVelocity.x = (mVelocity.x * resultVelocity.x < 0.0f) ? 0.0f : resultVelocity.x;
    mVelocity.z = (mVelocity.z * resultVelocity.z < 0.0f) ? 0.0f : resultVelocity.z;
}

void Physics::UpdateGravity(const float deltaTime, const SimpleMath::Vector3& moveDir, const float speed) {
    if (IsOnGround()) {
        return;
    }

    SimpleMath::Vector3 dragForce = SimpleMath::Vector3::Up * mFactor.dragCoeffi * speed * speed;
    SimpleMath::Vector3 dragAcceleration = dragForce / mFactor.mass.Count();

    // 최종 가속도 = 중력 + 공기 저항
    SimpleMath::Vector3 acceleration = SimpleMath::Vector3::Down * GRAVITY_ACCELERATION.Count() + dragAcceleration;
    mVelocity += acceleration * deltaTime;
}
