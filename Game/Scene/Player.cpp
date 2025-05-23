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
	mEmpty = false;

	DirectX::BoundingBox box{ {0.f,0.8f,0.f}, {0.25f, 0.8f, 0.25f} };
	mCollider = Collider{ box };

}

Player::Player(Mesh* mesh, GraphicsShaderBase* shader, MaterialIndex material, AnimatorGraph::AnimationGraphController animController) {
	mMesh = mesh;
	mShader = shader;
	mMaterial = material;
	mAnimController = animController;

	mActiveState = true;
	mEmpty = false;

	DirectX::BoundingBox box{ {0.f,0.8f,0.f}, {0.25f, 0.8f, 0.25f} };
	mCollider = Collider{ box };
}

bool Player::GetActiveState() const {
	return mActiveState;
}

void Player::SetActiveState(bool state) {
	mActiveState = state; 
}

bool Player::GetEmpty() const {
	return mEmpty;
}

void Player::SetEmpty(bool state) {
	mEmpty = state; 
}

void Player::AddEquipment(EquipmentObject equipment) {
	mEquipments.emplace_back(equipment);
}

void Player::ForwardUpdate() {
	if (mMyPlayer and not mRotateLock) {
		static const SimpleMath::Matrix localRotations[] = {
			SimpleMath::Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(45.f), 0.f, 0.f),	// 상 or 하 + 우 
			SimpleMath::Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(-45.f), 0.f, 0.f),	// 상 or 하 + 좌
			SimpleMath::Matrix::Identity
		};

		auto& keyboard = Input.GetKeyboardState();
		if (keyboard.W) {
			if (keyboard.A and not keyboard.D) {
				mTransform.SetLocalTransform(localRotations[1]);
			}
			else if (keyboard.D and not keyboard.A) {
				mTransform.SetLocalTransform(localRotations[0]);
			}
			else {
				mTransform.SetLocalTransform(localRotations[2]);
			}
		}
		else if (keyboard.S) {
			if (keyboard.A and not keyboard.D) {
				mTransform.SetLocalTransform(localRotations[0]);
			}
			else if (keyboard.D and not keyboard.A) {
				mTransform.SetLocalTransform(localRotations[1]);
			}
			else {
				mTransform.SetLocalTransform(localRotations[2]);
			}
		}
		else {
			mTransform.SetLocalTransform(localRotations[2]);
		}

		const float XSensivity = 0.15f;
		mTransform.Rotate(0.f, Input.GetDeltaMouseX() * Time.GetSmoothDeltaTime<float>() * XSensivity, 0.f);
	}

	mTransform.Update(Time.GetDeltaTime<float>());

}

void Player::Update(MeshRenderManager& manager) {
	static BoneTransformBuffer boneTransformBuffer{};

	if (mBoneMaskController.GetActiveState()) {
		mBoneMaskController.Update(Time.GetDeltaTime(), boneTransformBuffer);
	}
	else if (mAnimController.GetActiveState()) {
		mAnimController.Update(Time.GetDeltaTime(), boneTransformBuffer);
	}

	mTransform.UpdateWorldMatrix();
	mModelContext.world = mTransform.GetWorldMatrix();

	mCollider.UpdateBox(mTransform.GetWorldMatrix());

	manager.AppendBonedMeshContext(mShader, mMesh, ModelContext{mTransform.GetWorldMatrix().Transpose(), mCollider.GetCenter(), mCollider.GetExtents(), mMaterial}, boneTransformBuffer);
	manager.AppendShadowBonedMeshContext(mShader, mMesh, ModelContext{mTransform.GetWorldMatrix().Transpose(), mCollider.GetCenter(), mCollider.GetExtents(), mMaterial}, boneTransformBuffer);

	for (auto& equipment : mEquipments) {
		if (false == equipment.GetActiveState()) {
			continue; 
		}
		equipment.UpdateShaderVariables(boneTransformBuffer, mTransform.GetWorldMatrix());
		auto [mesh, shader, ModelContext] = equipment.GetRenderData();
		manager.AppendPlaneMeshContext(shader, mesh, ModelContext);
		manager.AppendShadowPlaneMeshContext(shader, mesh, ModelContext, 0);
	}

}

Transform& Player::GetTransform() {
	return mTransform; 
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

void Player::SetAnimation(Packets::AnimationState state) {
	if (mBoneMaskController.GetActiveState()) {
		mBoneMaskController.Transition(static_cast<size_t>(state));
		return; 
	} 


	if (mAnimController.GetActiveState()) {
		mAnimController.Transition(static_cast<size_t>(state)); 
		return; 
	}
}

void Player::SetMyPlayer() {
	mMyPlayer = true;
}

Player Player::Clone() {
	Player result{};

	result.mMesh = mMesh;
	result.mShader = mShader;
	result.mMaterial = mMaterial;

	if (mBoneMaskController.GetActiveState()) {
		result.mBoneMaskController = mBoneMaskController;
	}
	else if (mAnimController.GetActiveState()) {
		result.mAnimController = mAnimController;
	}

	result.mModelContext = mModelContext;
	result.mTransform = mTransform;

	if (mCollider.GetActiveState()) {
		result.mCollider = mCollider;
	}

	result.mEquipments = mEquipments;
	result.mActiveState = mActiveState;
	result.mMyPlayer = mMyPlayer;

	return result;
}

void Player::LockRotate(bool state) {
	mRotateLock = state; 
}

bool Player::GetRotateState() const {
	return mRotateLock;
}
