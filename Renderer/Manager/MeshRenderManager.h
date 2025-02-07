#pragma once
#include <unordered_map>
#include <vector>
#include "../Resource/DefaultBuffer.h"
#include "../Core/Shader.h"
#include "../Resource/PlaneMesh.h"

class MeshRenderManager {
	template<typename T>
	static constexpr T MAX_INSTANCE_COUNT = static_cast<T>(1024);
public:
	MeshRenderManager(); 
	~MeshRenderManager() = default;
public:
	void AppendPlaneMeshContext(GraphicsShaderBase* shader, PlaneMesh* mesh, SimpleMath::Matrix& world);

	void Render(ComPtr<ID3D12GraphicsCommandList> commandList); 
private:
	DefaultBuffer mPlaneMeshBuffer{};
	// DefaultBuffer mBonedMeshBuffer{};

	std::unordered_map<UINT,std::vector<std::pair<PlaneMesh*, std::vector<SimpleMath::Matrix>>>> mPlaneMeshContexts{};
};