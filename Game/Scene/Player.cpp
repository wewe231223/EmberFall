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

	DirectX::BoundingBox box{ {0.f,0.8f,0.f}, {0.25f, 0.8f, 0.25f} };
	mCollider = Collider{ box };

	mWeapon.SetActiveState(false);
}

bool Player::GetActiveState() const {
	return mActiveState;
}

void Player::SetActiveState(bool state) {
	mActiveState = false; 
}

void Player::AddEquipment(EquipmentObject equipment) {
	mEquipments.emplace_back(equipment);
}

void Player::Update(std::shared_ptr<MeshRenderManager>& manager) {

	if (mMyPlayer) {
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

	static BoneTransformBuffer boneTransformBuffer{};

	mBoneMaskController.Update(Time.GetDeltaTime(), boneTransformBuffer);

	mTransform.Update(Time.GetDeltaTime<float>());
	mTransform.UpdateWorldMatrix();
	mModelContext.world = mTransform.GetWorldMatrix();

	mCollider.UpdateBox(mTransform.GetWorldMatrix());

	manager->AppendBonedMeshContext(mShader, mMesh, ModelContext{mTransform.GetWorldMatrix().Transpose(), mCollider.GetCenter(), mCollider.GetExtents(), mMaterial}, boneTransformBuffer);

	for (auto& equipment : mEquipments) {
		if (false == equipment.GetActiveState()) {
			continue; 
		}
		equipment.UpdateShaderVariables(boneTransformBuffer, mTransform.GetWorldMatrix());
		auto [mesh, shader, ModelContext] = equipment.GetRenderData();
		manager->AppendPlaneMeshContext(shader, mesh, ModelContext);
		manager->AppendShadowPlaneMeshContext(shader, mesh, ModelContext);
	}

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

void Player::SetMyPlayer() {
	mMyPlayer = true;
}
