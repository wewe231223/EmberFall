#pragma once
#include <unordered_map>
#include <vector>
#include "../Resource/DefaultBuffer.h"
#include "../Resource/Mesh.h"
#include "../Core/Shader.h"
#include "../Utility/Defines.h"
#include "../Config/Config.h"

#include "../External/Include/absl/container/flat_hash_map.h"
#ifdef max 
#undef max
#endif

//#define RENDER_BB 

class MeshRenderManager {
public:
	template<typename T>
	static constexpr T MAX_INSTANCE_COUNT = static_cast<T>(10000);

	template<typename T> 
	static constexpr T MAX_BONE_COUNT = static_cast<T>(MAX_INSTANCE_COUNT<T> * Config::MAX_BONE_COUNT_PER_INSTANCE<T>);

	static constexpr UINT RESERVED_CONTEXT_SLOT = 8;
public:
	MeshRenderManager() = default;
	MeshRenderManager(ComPtr<ID3D12Device> device);
	~MeshRenderManager() = default;

	MeshRenderManager(const MeshRenderManager& other) = delete;
	MeshRenderManager& operator=(const MeshRenderManager& other) = delete;

	MeshRenderManager(MeshRenderManager&& other) = default;
	MeshRenderManager& operator=(MeshRenderManager&& other) = default;
public:
	void AppendPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, UINT reservedSlot = std::numeric_limits<UINT>::max() );
	void AppendBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, BoneTransformBuffer& boneTransforms );

	void AppendShadowPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, UINT reservedSlot = std::numeric_limits<UINT>::max());
	void AppendShadowBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, BoneTransformBuffer& boneTransforms);


	void PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList);
	
	void RenderShadowPass(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex,D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderGPass(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void Reset(); 
private:
	void RenderShadowPassPlainMesh(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderShadowPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);

	void RenderGPassPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderGPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
private:
	DefaultBuffer mPlainMeshBuffer{};

	DefaultBuffer mBonedMeshBuffer{}; 
	DefaultBuffer mAnimationBuffer{};

	DefaultBuffer mShadowPlainMeshBuffer{};

	DefaultBuffer mShadowBonedMeshBuffer{};
	DefaultBuffer mShadowAnimationBuffer{};

	UINT mBoneCounter{ 0 };
	UINT mShadowBoneCounter{ 0 };
	UINT mReservedSlotCounter{ 0 };

	std::array<UINT, 2> mShadowMeshCounter{ 0, 0 };

	std::vector<SimpleMath::Matrix> mBoneTransforms{};
	std::vector<SimpleMath::Matrix> mShadowBoneTransforms{};
	absl::flat_hash_map<GraphicsShaderBase*, absl::flat_hash_map<Mesh*, std::vector<AnimationModelContext>>> mBonedMeshContexts{};
	absl::flat_hash_map<GraphicsShaderBase*, absl::flat_hash_map<Mesh*, std::vector<AnimationModelContext>>> mShadowBonedMeshContexts{};

	absl::flat_hash_map<GraphicsShaderBase*, absl::flat_hash_map<Mesh*, std::vector<ModelContext>>> mPlainMeshReserved{};
	absl::flat_hash_map<GraphicsShaderBase*, absl::flat_hash_map<Mesh*, std::vector<ModelContext>>> mPlainMeshContexts{};

	std::array<absl::flat_hash_map<GraphicsShaderBase*, absl::flat_hash_map<Mesh*, std::vector<ModelContext>>>, 2> mShadowPlainMeshContexts{};

	std::unique_ptr<GraphicsShaderBase> mSkeletonBoundingboxRenderShader{};
	std::unique_ptr<GraphicsShaderBase> mStandardBoundingBoxRenderShader{};

};