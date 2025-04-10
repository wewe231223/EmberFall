#include "pch.h"
#include "GameObject.h"
#include "../Game/System/Timer.h"

GameObject::operator bool() const {
	return mActiveState;
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
	mTransform.Update(Time.GetDeltaTime<float>());
	mTransform.UpdateWorldMatrix();

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mModelContext.world = mTransform.GetWorldMatrix();

	if (mGraphController) {
		mGraphController.Update(Time.GetDeltaTime(), boneTransformBuffer);
	}
	else if (mBoneMaskGraphController) {
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
