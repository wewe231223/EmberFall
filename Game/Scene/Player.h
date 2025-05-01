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
	~Player() = default;

	Player(const Player&) = default;
	Player& operator=(const Player&) = default;

	Player(Player&&) = default;
	Player& operator=(Player&&) = default;
public:
	bool GetActiveState() const;
	void SetActiveState(bool state);

	void AddEquipment(EquipmentObject equipment);

	void Update(std::shared_ptr<MeshRenderManager>&);

	Transform& GetTransform();
	AnimatorGraph::BoneMaskAnimationGraphController& GetBoneMaskController();

	void SetMesh(Mesh* mesh);
	void SetShader(GraphicsShaderBase* shader);
	void SetMaterial(MaterialIndex material);

	void SetBoneMaskController(AnimatorGraph::BoneMaskAnimationGraphController boneMaskController);

	void SetMyPlayer();
	
private:
	Mesh* mMesh{};
	GraphicsShaderBase* mShader{};
	MaterialIndex mMaterial{ 0 };
	std::vector<EquipmentObject> mEquipments{}; 

	AnimatorGraph::BoneMaskAnimationGraphController mBoneMaskController{};

	ModelContext mModelContext{};
	Transform mTransform{}; 

	Collider mCollider{};

	bool mActiveState{ false };
	bool mMyPlayer{ false };
};