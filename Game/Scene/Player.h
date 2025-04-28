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

	void AddEquipment(EquipmentObject equipment);

	void Update(MeshRenderManager& manager);

	Transform& GetTransform();

	void SetMesh(Mesh* mesh);
	void SetShader(GraphicsShaderBase* shader);
	void SetMaterial(MaterialIndex material);

	void SetAnimation(Packets::AnimationState state);

	void SetMyPlayer();
	
private:
	Mesh* mMesh{};
	GraphicsShaderBase* mShader{};
	MaterialIndex mMaterial{ 0 };
	std::vector<EquipmentObject> mEquipments{}; 

	AnimatorGraph::BoneMaskAnimationGraphController mBoneMaskController{};
	AnimatorGraph::AnimationGraphController mAnimController{}; 

	ModelContext mModelContext{};
	Transform mTransform{}; 

	Collider mCollider{};

	bool mActiveState{ false };
	bool mMyPlayer{ false };
};