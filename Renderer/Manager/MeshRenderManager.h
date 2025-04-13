#pragma once
#include <unordered_map>
#include <vector>
#include "../Resource/DefaultBuffer.h"
#include "../Resource/Mesh.h"
#include "../Core/Shader.h"
#include "../Utility/Defines.h"
#include "../Config/Config.h"

#ifdef max 
#undef max
#endif


#define RENDER_BB 

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


	void PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList);
	
	void RenderShadowPass(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex,D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderGPass(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void Reset(); 
private:
	void RenderShadowPassPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderShadowPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);

	void RenderGPassPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
	void RenderGPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera);
private:
	DefaultBuffer mPlainMeshBuffer{};

	DefaultBuffer mBonedMeshBuffer{}; 
	DefaultBuffer mAnimationBuffer{};

	DefaultBuffer mShadowPlainMeshBuffer{};


	UINT mBoneCounter{ 0 };
	UINT mReservedSlotCounter{ 0 };

	std::vector<SimpleMath::Matrix> mBoneTransforms{};
	std::unordered_map<GraphicsShaderBase*, std::unordered_map<Mesh*, std::vector<AnimationModelContext>>> mBonedMeshContexts{};
	
	std::unordered_map<GraphicsShaderBase*, std::unordered_map<Mesh*, std::vector<ModelContext>>> mPlainMeshReserved{};
	std::unordered_map<GraphicsShaderBase* ,std::unordered_map<Mesh*, std::vector<ModelContext>>> mPlainMeshContexts{};

	std::unordered_map<GraphicsShaderBase*, std::unordered_map<Mesh*, std::vector<ModelContext>>> mShadowPlainMeshContexts{};

	std::unique_ptr<GraphicsShaderBase> mSkeletonBoundingboxRenderShader{};
	std::unique_ptr<GraphicsShaderBase> mStandardBoundingBoxRenderShader{}; 
};