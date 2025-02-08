#pragma once
#include <unordered_map>
#include <vector>
#include "../Resource/DefaultBuffer.h"
#include "../Core/Shader.h"
#include "../Resource/PlainMesh.h"


class MeshRenderManager {
	template<typename T>
	static constexpr T MAX_INSTANCE_COUNT = static_cast<T>(4096);
public:
	MeshRenderManager(ComPtr<ID3D12Device> device);
	~MeshRenderManager() = default;
public:
	void AppendPlaneMeshContext(GraphicsShaderBase* shader, PlainMesh* mesh, const SimpleMath::Matrix& world);

	void Render(ComPtr<ID3D12GraphicsCommandList> commandList); 
	void Reset(); 
private:
	DefaultBuffer mPlaneMeshBuffer{};
	// DefaultBuffer mBonedMeshBuffer{};

	std::unordered_map<GraphicsShaderBase* ,std::unordered_map<PlainMesh*, std::vector<SimpleMath::Matrix>>> mPlainMeshContexts{};
};