#include "pch.h"
#include "GameObject.h"

GameObject::operator bool() const {
	return mActiveState;
}

std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GameObject::GetRenderData() const {
	return std::make_tuple(mMesh, mShader, ModelContext{ mTransform.GetWorldMatrix().Transpose(), mMaterial});
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

void GameObject::UpdateShaderVariables(){
	mTransform.UpdateWorldMatrix();
	mModelContext.world = mTransform.GetWorldMatrix();
}
