#pragma once 
#include <tuple> 
#include "../Renderer/Resource/Mesh.h"
#include "../Renderer/Core/Shader.h"
#include "../Utility/Defines.h"
#include "../Game/GameObject/Transform.h"
#include "../GameObject/Collider.h"


class EquipmentObject {
public:
	EquipmentObject() = default;
	~EquipmentObject() = default;

	EquipmentObject(const EquipmentObject&) = default;
	EquipmentObject& operator=(const EquipmentObject&) = default;

	EquipmentObject(EquipmentObject&&) = default;
	EquipmentObject& operator=(EquipmentObject&&) = default;
public:
	bool GetActiveState() const; 

	std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GetRenderData() const;

	const Transform& GetTransform() const;
	Transform& GetTransform();

	void SetActiveState(bool state);
	void ToggleActiveState();

	void UpdateShaderVariables(BoneTransformBuffer& boneTransform, SimpleMath::Matrix& worldTransform);

	EquipmentObject Clone();
public:
	Mesh* mMesh{ nullptr };
	GraphicsShaderBase* mShader{ nullptr };
	MaterialIndex mMaterial{ 0 };

	UINT mEquipJointIndex{ 0 };

	Collider mCollider{};
private:
	ModelContext mModelContext{};

	Transform mTransform{};

	bool mActiveState{ false };
};