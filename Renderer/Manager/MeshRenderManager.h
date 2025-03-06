#pragma once
#include <unordered_map>
#include <vector>
#include "../Resource/DefaultBuffer.h"
#include "../Resource/Mesh.h"
#include "../Core/Shader.h"
#include "../Utility/Defines.h"

class MeshRenderManager {
	template<typename T>
	static constexpr T MAX_INSTANCE_COUNT = static_cast<T>(65535);

	template<typename T> 
	static constexpr T MAX_BONE_COUNT = static_cast<T>(MAX_INSTANCE_COUNT<T> * 100);
public:
	MeshRenderManager() = default;
	MeshRenderManager(ComPtr<ID3D12Device> device);
	~MeshRenderManager() = default;

	MeshRenderManager(const MeshRenderManager& other) = delete;
	MeshRenderManager& operator=(const MeshRenderManager& other) = delete;

	MeshRenderManager(MeshRenderManager&& other) = default;
	MeshRenderManager& operator=(MeshRenderManager&& other) = default;
public:
	void AppendPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world);
	void AppendBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, std::vector<SimpleMath::Matrix>& boneTransform);

	void PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Render(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void Reset(); 
private:
	void RenderPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
private:
	DefaultBuffer mPlainMeshBuffer{};

	DefaultBuffer mBonedMeshBuffer{}; 
	DefaultBuffer mAnimationBuffer{};

	UINT mBoneCounter{ 0 };

	std::vector<SimpleMath::Matrix> mBoneTransforms{};
	std::unordered_map<GraphicsShaderBase*, std::unordered_map<Mesh*, std::vector<AnimationModelContext>>> mBonedMeshContexts{};
	

	std::unordered_map<GraphicsShaderBase* ,std::unordered_map<Mesh*, std::vector<ModelContext>>> mPlainMeshContexts{};
};