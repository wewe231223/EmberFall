#include "pch.h"
#include "EquipmentObject.h"

bool EquipmentObject::GetActiveState() const {
	return mActiveState;
}

std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> EquipmentObject::GetRenderData() const {
	return std::make_tuple(mMesh, mShader, ModelContext{ mTransform.GetWorldMatrix().Transpose(), mCollider.GetCenter() , mCollider.GetExtents() ,mMaterial });
}

const Transform& EquipmentObject::GetTransform() const {
	return mTransform; 
}

Transform& EquipmentObject::GetTransform() {
	return mTransform; 
}

void EquipmentObject::SetActiveState(bool state) {
	mActiveState = state;
}

void EquipmentObject::ToggleActiveState() {
	mActiveState = !mActiveState;
}

void EquipmentObject::UpdateShaderVariables(BoneTransformBuffer& boneTransform, SimpleMath::Matrix& worldTransform) {
	mTransform.SetLocalTransform(boneTransform.boneTransforms[mEquipJointIndex].Transpose());
	mTransform.UpdateWorldMatrix(worldTransform);

	if (mCollider.GetActiveState()) {
		mCollider.UpdateBox(mTransform.GetWorldMatrix());
	}

	mModelContext.world = mTransform.GetWorldMatrix();
}

EquipmentObject EquipmentObject::Clone() {
	EquipmentObject clone{};

	clone.mMesh = mMesh;
	clone.mShader = mShader;
	clone.mMaterial = mMaterial;
	clone.mEquipJointIndex = mEquipJointIndex;
	clone.mCollider = mCollider;
	clone.mActiveState = mActiveState;
	clone.mModelContext = ModelContext{};
	clone.mTransform = Transform{};

	return clone; 
}
