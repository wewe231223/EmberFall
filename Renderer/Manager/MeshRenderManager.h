#pragma once
#include <unordered_map>
#include <vector>
#include "../Resource/DefaultBuffer.h"
#include "../Resource/PlainMesh.h"
#include "../Core/Shader.h"
#include "../Utility/Defines.h"

class MeshRenderManager {
	template<typename T>
	static constexpr T MAX_INSTANCE_COUNT = static_cast<T>(65535);
public:
	MeshRenderManager() = default;
	MeshRenderManager(ComPtr<ID3D12Device> device);
	~MeshRenderManager() = default;

	MeshRenderManager(const MeshRenderManager& other) = delete;
	MeshRenderManager& operator=(const MeshRenderManager& other) = delete;

	MeshRenderManager(MeshRenderManager&& other) = default;
	MeshRenderManager& operator=(MeshRenderManager&& other) = default;
public:
	void AppendPlaneMeshContext(GraphicsShaderBase* shader, PlainMesh* mesh, const PlainModelContext& world);

	void PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Render(ComPtr<ID3D12GraphicsCommandList> commandList); 
	void Reset(); 
private:
	DefaultBuffer mPlainMeshBuffer{};
	// DefaultBuffer mBonedMeshBuffer{};

	std::unordered_map<GraphicsShaderBase* ,std::unordered_map<PlainMesh*, std::vector<PlainModelContext>>> mPlainMeshContexts{};
};