#pragma once 
#include <tuple> 
#include "../Renderer/Resource/Mesh.h"
#include "../Renderer/Core/Shader.h"
#include "../Utility/Defines.h"
#include "../Game/GameObject/Transform.h"
#include "../GameObject/Collider.h"
#include "../GameObject/Animator.h"

#define MONO_TYPE_MESH

#ifdef MONO_TYPE_MESH 

class GameObject {
public:
	GameObject() = default;
public:
	bool GetActiveState() const;
	bool GetEmpty() const; 
	void SetEmpty(bool state); 

	std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GetRenderData() const;
	std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GetAnimationRenderData() const;

	const Transform& GetTransform() const;
	Transform& GetTransform();

	void SetActiveState(bool state); 
	void ToggleActiveState();

	void ForwardUpdate();

	void UpdateShaderVariables(); 
	void UpdateShaderVariables(SimpleMath::Matrix& parent);
	void UpdateShaderVariables(BoneTransformBuffer& boneTransformBuffer);

	bool GetAnimatorState() const;

	GameObject Clone(); 

	AnimatorGraph::AnimationGraphController& GetAnimationController();
public:
	Mesh* mMesh{ nullptr };
	GraphicsShaderBase* mShader{ nullptr };
	MaterialIndex mMaterial{ 0 };

	bool mAnimated{ false };
	AnimatorGraph::AnimationGraphController mGraphController{};
	AnimatorGraph::BoneMaskAnimationGraphController mBoneMaskGraphController{};

	Collider mCollider{};

	std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<size_t>> mBoneTransforms;
private:
	ModelContext mModelContext{};

	Transform mTransform{};

	bool mActiveState{ false };
	bool mEmpty{ true }; 
};


struct LODGroup {
	Mesh* mMesh{ nullptr };
	GraphicsShaderBase* mShader{ nullptr };
	MaterialIndex mMaterial{ 0 };
	float mDistanceSquared{ std::numeric_limits<float>::max() };
};

class LODGameObject {
public:
	LODGameObject() = default;

	LODGameObject(const LODGameObject&);
	LODGameObject& operator=(const LODGameObject&);

	LODGameObject(LODGameObject&&) noexcept;
	LODGameObject& operator=(LODGameObject&&) noexcept;
public:
	bool GetActiveState() const;
	bool GetEmpty() const;
	void SetEmpty(bool state);

	std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GetRenderData() const;

	const Transform& GetTransform() const;
	Transform& GetTransform();

	void SetActiveState(bool state);
	void ToggleActiveState();

	void UpdateLODLevel(const SimpleMath::Vector3& pos);
	void UpdateShaderVariables();

	LODGameObject Clone();
public:
	std::array<LODGroup, 4> mLODGroups{};
	Collider mCollider{}; 
private:
	ModelContext mModelContext{};

	Transform mTransform{};

	UINT mCurrentLODLevel{ 0 };

	bool mActiveState{ false };
	bool mEmpty{ true };
};


#else 


#endif 