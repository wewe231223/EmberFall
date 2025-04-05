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
	operator bool() const;

	std::tuple<Mesh*, GraphicsShaderBase*, ModelContext> GetRenderData() const;

	const Transform& GetTransform() const;
	Transform& GetTransform();

	void SetActiveState(bool state); 
	void ToggleActiveState();

	void UpdateShaderVariables(); 
	void UpdateShaderVariables(SimpleMath::Matrix& parent);
	void UpdateShaderVariables(BoneTransformBuffer& boneTransformBuffer);

	bool GetAnimatorState() const;

	GameObject Clone(); 
public:
	Mesh* mMesh{ nullptr };
	GraphicsShaderBase* mShader{ nullptr };
	MaterialIndex mMaterial{ 0 };

	bool mAnimated{ false };
	AnimatorGraph::AnimationGraphController mGraphController{};
	AnimatorGraph::BoneMaskAnimationGraphController mBoneMaskGraphController{};

	Collider mCollider{};
private:
	ModelContext mModelContext{};

	Transform mTransform{};

	bool mActiveState{ false };
};
#else 


#endif 