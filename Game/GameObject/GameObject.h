#pragma once 
#include <tuple> 
#include "../Renderer/Resource/PlainMesh.h"
#include "../Renderer/Core/Shader.h"
#include "../Utility/Defines.h"
#include "../Game/GameObject/Transform.h"
#include "../GameObject/Collider.h"
#define MONO_TYPE_MESH

#ifdef MONO_TYPE_MESH 
class GameObject {
public:
	GameObject() = default;
public:
	operator bool() const;

	std::tuple<PlainMesh*, GraphicsShaderBase*, PlainModelContext> GetRenderData() const;

	const Transform& GetTransform() const;
	Transform& GetTransform();

	void SetActiveState(bool state); 
	void ToggleActiveState();
private:
	PlainMesh* mMesh{ nullptr };
	GraphicsShaderBase* mShader{ nullptr };
	MaterialIndex mMaterial{ 0 };

	Transform mTransform{};
	Collider mCollider{};

	bool mActiveState{};
};
#else 


#endif 