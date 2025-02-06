#include "pch.h"
#include "Transform.h"

Transform::Transform() { }

Transform::~Transform() { }

Transform::Transform(const Transform& other) 
    : mPosition{ other.mPosition }, mRotation{ other.mRotation }, mScale{ other.mScale } { }

Transform::Transform(Transform&& other) noexcept
    : mPosition{ other.mPosition }, mRotation{ other.mRotation }, mScale{ other.mScale } { }

Transform& Transform::operator=(const Transform& other) {
    mPosition = other.mPosition;
    mRotation = other.mRotation;
    mScale = other.mScale;

    return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept {
    mPosition = other.mPosition;
    mRotation = other.mRotation;
    mScale = other.mScale;

    return *this;
}

SimpleMath::Vector3 Transform::Forward() const
{
    return SimpleMath::Vector3();
}

SimpleMath::Vector3 Transform::Right() const
{
    return SimpleMath::Vector3();
}

SimpleMath::Vector3 Transform::Up() const
{
    return SimpleMath::Vector3();
}

SimpleMath::Vector3 Transform::GetPosition() const {
    return mPosition;
}

SimpleMath::Quaternion Transform::GetRotation() const {
    return mRotation;
}

SimpleMath::Vector3 Transform::GetScale() const {
    return mScale;
}

SimpleMath::Matrix Transform::GetWorld() const {
    return mWorld;
}

void Transform::SetY(const float y) {
    mPosition.y = y;
}

void Transform::Translate(const SimpleMath::Vector3& v) {
    mPosition += v;
}

void Transform::Move(const SimpleMath::Vector3& moveVec) {
    Translate(SimpleMath::Vector3::Transform(moveVec, mRotation));
}

void Transform::Rotation(const SimpleMath::Quaternion& quat) {
    mRotation = quat;
}

void Transform::Rotate(const float yaw, const float pitch, const float roll) {
    auto yawPitchRoll = SimpleMath::Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
    mRotation = SimpleMath::Quaternion::Concatenate(yawPitchRoll, mRotation);
}

void Transform::Rotate(const SimpleMath::Vector3& v) {
    auto yawPitchRoll = SimpleMath::Quaternion::CreateFromYawPitchRoll(v.x, v.y, v.z);
    mRotation = SimpleMath::Quaternion::Concatenate(yawPitchRoll, mRotation);
}

void Transform::Rotate(const SimpleMath::Quaternion& quat) {
    mRotation = SimpleMath::Quaternion::Concatenate(quat, mRotation);
}

void Transform::RotateSmoothly(const SimpleMath::Quaternion& quat) {

}

void Transform::Scale(const SimpleMath::Vector3& v) {
    mScale = v;
}

void Transform::Update() {
    mWorld = SimpleMath::Matrix::CreateScale(mScale) 
        * SimpleMath::Matrix::CreateFromQuaternion(mRotation)
        * SimpleMath::Matrix::CreateTranslation(mPosition);
}
