#include "pch.h"
#include "Transform.h"

Transform::Transform(const SimpleMath::Vector3& pos) : mPosition(pos) {

}

Transform::Transform(const SimpleMath::Vector3& pos, const SimpleMath::Quaternion& rot) : mPosition(pos), mRotation(rot) {

}

Transform::Transform(const SimpleMath::Vector3& pos, const SimpleMath::Quaternion& rot, const SimpleMath::Vector3& scale) : mPosition(pos), mRotation(rot), mScale(scale) {

}

const SimpleMath::Vector3& Transform::GetPosition() const {
	return mPosition;
}

const SimpleMath::Quaternion& Transform::GetRotation() const {
    return mRotation;
}

const SimpleMath::Vector3& Transform::GetScale() const {
    return mScale; 
}

const SimpleMath::Matrix& Transform::GetWorldMatrix() const {
    return mWorldMatrix; 
}

const SimpleMath::Vector3 Transform::GetForward() const {
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::UnitZ, mRotation);
}

const SimpleMath::Vector3 Transform::GetRight() const {
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::UnitX, mRotation);
}

const SimpleMath::Vector3 Transform::GetUp() const {
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::UnitY, mRotation);
}

const Transform& Transform::GetChild(int index) const {
    return Transform(); // TEMP
}

SimpleMath::Vector3& Transform::GetPosition() {
    return mPosition;
}

SimpleMath::Quaternion& Transform::GetRotation() {
    return mRotation;
}

SimpleMath::Vector3& Transform::GetScale() {
    return mScale;
}

SimpleMath::Matrix& Transform::GetWorldMatrix() {
	return mWorldMatrix;
}

Transform& Transform::GetChild(int index) {
    Transform ret{}; 
	return ret; // TEMP
}

void Transform::Translate(const SimpleMath::Vector3& offset) {
    mPosition += offset;
}

void Transform::SetPosition(const SimpleMath::Vector3& pos) {
	mPosition = pos;
}

void Transform::SetSpeed(float speed) {
	mSpeed = speed; 
}

void Transform::SetDirection(const SimpleMath::Vector3& dir) {
	mDirection = dir;
}

void Transform::Scaling(const SimpleMath::Vector3& scale) {
	mScale *= scale;
}

void Transform::Scaling(float x, float y, float z) {
	mScale.x *= x;
	mScale.y *= y;
	mScale.z *= z;
}

void Transform::Rotate(float pitch, float yaw, float roll) {
	mRotation = SimpleMath::Quaternion::Concatenate(SimpleMath::Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll), mRotation);
	mRotation.Normalize(); 
}

void Transform::Look(const Transform& target) {
	DirectX::SimpleMath::Vector3 direction = target.GetPosition() - mPosition;
	direction.Normalize();

	auto look = DirectX::SimpleMath::Quaternion::FromToRotation(Transform::GetForward(), direction);
	look.Normalize();
	mRotation = DirectX::SimpleMath::Quaternion::Concatenate(look, mRotation);

	mRotation.Normalize();
}

void Transform::Look(const SimpleMath::Vector3& target) {
	auto diretion = target - mPosition;
	diretion.Normalize();

	auto look = SimpleMath::Quaternion::FromToRotation(Transform::GetForward(), diretion);
	look.Normalize(); 

	mRotation = SimpleMath::Quaternion::Concatenate(look, mRotation);
	mRotation.Normalize();
}

Transform Transform::CreateChild(const SimpleMath::Vector3& localPosition, const SimpleMath::Quaternion& localRotate, const SimpleMath::Vector3& localScale) {
    return Transform();
}

void Transform::SetLocalTransform(const SimpleMath::Matrix& localMatrix) {
	mLocalMatrix = localMatrix;
}

void Transform::Update(float deltaTime) {
	//mPosition += mDirection * mSpeed * deltaTime;
}

void Transform::UpdateWorldMatrix() {
	mWorldMatrix = mLocalMatrix * SimpleMath::Matrix::CreateScale(mScale) * SimpleMath::Matrix::CreateFromQuaternion(mRotation) * SimpleMath::Matrix::CreateTranslation(mPosition) ;
}

void Transform::UpdateWorldMatrix(SimpleMath::Matrix& parent) {
	mWorldMatrix = SimpleMath::Matrix::CreateScale(mScale) * SimpleMath::Matrix::CreateFromQuaternion(mRotation) * SimpleMath::Matrix::CreateTranslation(mPosition) * ( mLocalMatrix * parent );
}
