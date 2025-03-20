#include "pch.h"
#include "Player.h"
#include "../Game/System/Timer.h"
#include "../Game/System/Input.h"

Player::Player(Mesh* mesh, GraphicsShaderBase* shader, MaterialIndex material, AnimatorGraph::BoneMaskAnimationGraphController boneMaskController) {
	mMesh = mesh;
	mShader = shader;
	mMaterial = material;
	mBoneMaskController = boneMaskController;

	mActiveState = true;
}

bool Player::GetActiveState() const {
	return mActiveState;
}

void Player::Update(std::shared_ptr<MeshRenderManager>& manager) {
	const float XSensivity = 0.15f;

	mTransform.Rotate(0.f, Input.GetDeltaMouseX() * Time.GetSmoothDeltaTime<float>() * XSensivity, 0.f);


	static BoneTransformBuffer boneTransformBuffer{};

	mBoneMaskController.Update(Time.GetDeltaTime(), boneTransformBuffer);

	mTransform.UpdateWorldMatrix();
	mModelContext.world = mTransform.GetWorldMatrix();

	manager->AppendBonedMeshContext(mShader, mMesh, ModelContext{mTransform.GetWorldMatrix().Transpose(), SimpleMath::Vector3{0.3f, 0.8f, 0.3f}, mMaterial}, boneTransformBuffer);
}

Transform& Player::GetTransform() {
	return mTransform; 
}

AnimatorGraph::BoneMaskAnimationGraphController& Player::GetBoneMaskController() {
	return mBoneMaskController;
}

void Player::SetMesh(Mesh* mesh) {
	mMesh = mesh;
}

void Player::SetShader(GraphicsShaderBase* shader) {
	mShader = shader;
}

void Player::SetMaterial(MaterialIndex material) {
	mMaterial = material;
}

void Player::SetBoneMaskController(AnimatorGraph::BoneMaskAnimationGraphController boneMaskController) {
	mBoneMaskController = boneMaskController;
}
