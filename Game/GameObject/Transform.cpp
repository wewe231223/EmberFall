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

void Transform::SetPrediction(const SimpleMath::Vector3& target, const float time) {
	mPrevPos = mPosition; 
	mTargetPos = target; 

	mPredictTime = time;
	mCumulateTime = 0.f;

}

void Transform::ResetPrediction() {
	mPrevPos = mPosition; 
	mTargetPos = mPosition;
	mPredictTime = 0.f;
	mCumulateTime = 0.f;
}

void Transform::Scaling(const SimpleMath::Vector3& scale) {
	mScale *= scale;
}

void Transform::Scaling(float x, float y, float z) {
	mScale.x *= x;
	mScale.y *= y;
	mScale.z *= z;
}

void Transform::SetRotation(const SimpleMath::Quaternion& rotation) {
	mRotation = rotation;
	mRotation.Normalize();
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
	if (mPredictTime == 0.f) {
		return;
	}

	mCumulateTime += deltaTime; 
	float t = std::clamp(mCumulateTime / mPredictTime, 0.f, 1.f);

	mPosition = SimpleMath::Vector3::Lerp(mPrevPos, mTargetPos, t);
}

void Transform::UpdateWorldMatrix() {
	mWorldMatrix = mLocalMatrix * SimpleMath::Matrix::CreateScale(mScale) * SimpleMath::Matrix::CreateFromQuaternion(mRotation) * SimpleMath::Matrix::CreateTranslation(mPosition) ;
}

void Transform::UpdateWorldMatrix(SimpleMath::Matrix& parent) {
	mWorldMatrix = SimpleMath::Matrix::CreateScale(mScale) * SimpleMath::Matrix::CreateFromQuaternion(mRotation) * SimpleMath::Matrix::CreateTranslation(mPosition) * ( mLocalMatrix * parent );
}
