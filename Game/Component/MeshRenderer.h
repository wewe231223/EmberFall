#pragma once 
#include "../Game/Component/ComponentBase.h"
#include "../Renderer/Resource/PlainMesh.h"
#include "../Utility/Defines.h"
class MeshRenderer : public ComponentBase {
public:
	static constexpr size_t index = 1;
public:
	MeshRenderer() = default;
	MeshRenderer(PlainMesh mesh, MaterialIndex material) : mMesh{ mesh }, mMaterial{ material } {}
	virtual ~MeshRenderer() = default;

	MeshRenderer(const MeshRenderer& other) = default;
	MeshRenderer& operator=(const MeshRenderer& other) = default;

	MeshRenderer(MeshRenderer&& other) noexcept = default;
	MeshRenderer& operator=(MeshRenderer&& other) noexcept = default;
public:
	PlainMesh mMesh{};
	MaterialIndex mMaterial{};
};