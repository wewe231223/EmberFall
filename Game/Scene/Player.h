#pragma once 
#include "../Game/GameObject/GameObject.h"
#include "../Game/GameObject/Animator.h"
#include "../Game/GameObject/EquipmentObject.h"
#include "../Game/GameObject/Transform.h"
#include "../Renderer/Resource/Mesh.h"
#include "../Renderer/Core/Shader.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Utility/Defines.h"


class Player {
public:
	Player() = default;
	Player(Mesh* mesh, GraphicsShaderBase* shader, MaterialIndex material, AnimatorGraph::BoneMaskAnimationGraphController boneMaskController);
	Player(Mesh* mesh, GraphicsShaderBase* shader, MaterialIndex material, AnimatorGraph::AnimationGraphController animController);
	~Player() = default;

	Player(const Player&) = default;
	Player& operator=(const Player&) = default;

	Player(Player&&) = default;
	Player& operator=(Player&&) = default;
public:
	bool GetActiveState() const;
	void SetActiveState(bool state);
	
	bool GetEmpty() const; 
	void SetEmpty(bool state);

	void AddEquipment(EquipmentObject equipment);

	void ForwardUpdate(); 
	void Update(MeshRenderManager& manager);

	Transform& GetTransform();

	void SetMesh(Mesh* mesh);
	void SetShader(GraphicsShaderBase* shader);
	void SetMaterial(MaterialIndex material);

	void SetAnimation(Packets::AnimationState state);

	void SetMyPlayer();
	
	Player Clone();

	void LockRotate(bool state); 
	bool GetRotateState() const;

	void SetRole(Packets::EntityType role);
	Packets::EntityType GetMyRole() const;

private:
	Mesh* mMesh{};
	GraphicsShaderBase* mShader{};
	MaterialIndex mMaterial{ 0 };
	std::vector<EquipmentObject> mEquipments{}; 

	AnimatorGraph::BoneMaskAnimationGraphController mBoneMaskController{};
	AnimatorGraph::AnimationGraphController mAnimController{}; 

	std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<size_t>> mBoneTransforms;


	ModelContext mModelContext{};
	Transform mTransform{}; 

	Collider mCollider{};

	Packets::EntityType mMyRole{ Packets::EntityType::EntityType_MAX };

	bool mActiveState{ false };
	bool mEmpty{ true };

	bool mMyPlayer{ false };
	bool mRotateLock{ false };
};