#include "pch.h"
#include "GameObject.h"
#include "../Game/System/Timer.h"

bool GameObject::GetActiveState() const {
	return mActiveState;
}

bool GameObject::GetEmpty() const {
	return mEmpty;
}

void GameObject::SetEmpty(bool state) {
	mEmpty = state; 
}

std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GameObject::GetRenderData() const {
	return std::make_tuple(mMesh, mShader, ModelContext{ mTransform.GetWorldMatrix().Transpose(), mCollider.GetCenter(), mCollider.GetExtents() ,mMaterial});
}

const Transform& GameObject::GetTransform() const {
	return mTransform;
}

Transform& GameObject::GetTransform() {
	return mTransform;
}

void GameObject::SetActiveState(bool state) {
	mActiveState = state;
}

void GameObject::ToggleActiveState() {
	mActiveState = !mActiveState;
}

void GameObject::ForwardUpdate() {
	mTransform.Update(Time.GetDeltaTime<float>());
}

void GameObject::UpdateShaderVariables() {
	mTransform.Update(Time.GetDeltaTime<float>());
	mTransform.UpdateWorldMatrix();

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mModelContext.world = mTransform.GetWorldMatrix();
}

void GameObject::UpdateShaderVariables(SimpleMath::Matrix& parent) {
	mTransform.Update(Time.GetDeltaTime<float>());
	mTransform.UpdateWorldMatrix(parent);

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mModelContext.world = mTransform.GetWorldMatrix();
}

void GameObject::UpdateShaderVariables(BoneTransformBuffer& boneTransformBuffer) {
	mTransform.UpdateWorldMatrix();

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mModelContext.world = mTransform.GetWorldMatrix();

	if (mGraphController.GetActiveState()) {
		mGraphController.Update(Time.GetDeltaTime(), boneTransformBuffer);
	}
	else if (mBoneMaskGraphController.GetActiveState()) {
		mBoneMaskGraphController.Update(Time.GetDeltaTime(), boneTransformBuffer);
	}
}

bool GameObject::GetAnimatorState() const {
	return mAnimated; 
}

GameObject GameObject::Clone() {
	GameObject clone{}; 

	clone.mActiveState = true;
	clone.mAnimated = mAnimated;
	clone.mBoneMaskGraphController = mBoneMaskGraphController;
	clone.mCollider = mCollider;
	clone.mGraphController = mGraphController;
	clone.mMaterial = mMaterial;
	clone.mShader = mShader;
	clone.mMesh = mMesh;
	clone.mModelContext = ModelContext{};
	clone.mTransform = Transform{};
	
	return clone; 
}

AnimatorGraph::AnimationGraphController& GameObject::GetAnimationController() {
	return mGraphController; 
}

LODGameObject::LODGameObject(const LODGameObject& other) {
	mShader = other.mShader;
	mMaterial = other.mMaterial;
	mTransform = other.mTransform;
	mCollider = other.mCollider;
	
	std::copy(other.mLODGroups.begin(), other.mLODGroups.end(), mLODGroups.begin());
	
	mCurrentLODLevel = other.mCurrentLODLevel;
	mActiveState = other.mActiveState;
	mEmpty = other.mEmpty;
	mModelContext = other.mModelContext;
}

LODGameObject& LODGameObject::operator=(const LODGameObject& other) {
	if (this == &other) {
		return *this;
	}

	mShader = other.mShader;
	mMaterial = other.mMaterial;
	mTransform = other.mTransform;
	mCollider = other.mCollider;
	
	std::copy(other.mLODGroups.begin(), other.mLODGroups.end(), mLODGroups.begin());
	
	mCurrentLODLevel = other.mCurrentLODLevel;
	mActiveState = other.mActiveState;
	mEmpty = other.mEmpty;
	mModelContext = other.mModelContext;
	
	return *this;
}

LODGameObject::LODGameObject(LODGameObject&& other) {
	mShader = std::move(other.mShader);
	mMaterial = std::move(other.mMaterial);
	mTransform = std::move(other.mTransform);
	mCollider = std::move(other.mCollider);

	std::move(other.mLODGroups.begin(), other.mLODGroups.end(), mLODGroups.begin());

	mCurrentLODLevel = other.mCurrentLODLevel;
	mActiveState = other.mActiveState;
	mEmpty = other.mEmpty;
	mModelContext = std::move(other.mModelContext);
}

LODGameObject& LODGameObject::operator=(LODGameObject&& other) {
	if (this == &other) {
		return *this;
	}
	
	mShader = std::move(other.mShader);
	mMaterial = std::move(other.mMaterial);
	mTransform = std::move(other.mTransform);
	mCollider = std::move(other.mCollider);
	
	std::move(other.mLODGroups.begin(), other.mLODGroups.end(), mLODGroups.begin());
	
	mCurrentLODLevel = other.mCurrentLODLevel;
	mActiveState = other.mActiveState;
	mEmpty = other.mEmpty;
	mModelContext = std::move(other.mModelContext);

	return *this;
}

bool LODGameObject::GetActiveState() const {
	return mActiveState;
}

bool LODGameObject::GetEmpty() const {
	return mEmpty;
}

void LODGameObject::SetEmpty(bool state) {
	mEmpty = state;
}

std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> LODGameObject::GetRenderData() const {
	return { mLODGroups[mCurrentLODLevel].mMesh, mShader, ModelContext{ mTransform.GetWorldMatrix().Transpose(), mCollider.GetCenter(), mCollider.GetExtents(), mMaterial } };
}

const Transform& LODGameObject::GetTransform() const {
	return mTransform; 
}

Transform& LODGameObject::GetTransform() {
	return mTransform; 
}

void LODGameObject::SetActiveState(bool state) {
	mActiveState = state; 
}

void LODGameObject::ToggleActiveState() {
	mActiveState = !mActiveState;
}

void LODGameObject::UpdateLODLevel(const SimpleMath::Vector3& pos) {
	auto distanceSq = (pos - mTransform.GetPosition()).LengthSquared();

	mCurrentLODLevel = 0; 
	for (auto& lodGroup : mLODGroups) {
		if (distanceSq < lodGroup.mDistanceSquared or lodGroup.mMesh == nullptr) {
			break;
		} 
		++mCurrentLODLevel;
	}
}

void LODGameObject::UpdateShaderVariables() {
	mTransform.Update(Time.GetDeltaTime<float>());
	mTransform.UpdateWorldMatrix();

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mModelContext.world = mTransform.GetWorldMatrix();
}

LODGameObject LODGameObject::Clone() {
	LODGameObject clone{};
	clone.mActiveState = true;
	clone.mShader = mShader;
	clone.mMaterial = mMaterial;
	clone.mTransform = mTransform;
	clone.mCollider = mCollider;
	std::copy(mLODGroups.begin(), mLODGroups.end(), clone.mLODGroups.begin()); 

	clone.mCurrentLODLevel = mCurrentLODLevel;
	return clone;
}
