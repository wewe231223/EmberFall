#include "pch.h"
#include "GameObject.h"
#include "../Game/System/Timer.h"

GameObject::operator bool() const {
	return mActiveState;
}

std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GameObject::GetRenderData() const {
	return std::make_tuple(mMesh, mShader, ModelContext{ mTransform.GetWorldMatrix().Transpose(), mCollider.GetExtents() ,mMaterial});
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

void GameObject::UpdateShaderVariables(BoneTransformBuffer& boneTransform) {
	static double counter{ 0.0 };
	counter += 0.00000001;

	if (mAnimator.GetActivated()) {
		// mAnimator.UpdateBoneTransform(counter, boneTransform);
		mGraphAnimator.UpdateBoneTransform(0.00001 , boneTransform);
	}

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mTransform.UpdateWorldMatrix();
	mModelContext.world = mTransform.GetWorldMatrix();
}

bool GameObject::GetAnimatorState() const {
	return mAnimator.GetActivated();
}
