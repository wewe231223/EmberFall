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

	void UpdateShaderVariables(BoneTransformBuffer& boneTransform); 
	bool GetAnimatorState() const;
public:
	Mesh* mMesh{ nullptr };
	GraphicsShaderBase* mShader{ nullptr };
	MaterialIndex mMaterial{ 0 };

	Animator mAnimator{};
	Collider mCollider{};
private:
	ModelContext mModelContext{};

	Transform mTransform{};

	bool mActiveState{ true };
};
#else 


#endif 