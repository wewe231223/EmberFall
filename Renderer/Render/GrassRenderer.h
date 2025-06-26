#pragma once 
#include "../Utility/DirectXInclude.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"
#include "../Renderer/Core/Shader.h"
#include "../MeshLoader/Loader/TerrainLoader.h"

class GrassTree {
	struct Node {
		SimpleMath::Vector2 point;
		int axis;
		union {
			struct { int left, right; } internal;
			struct { size_t begin, end; } leaf;
		};
	};

	static constexpr size_t LEAF_SIZE = 128;

public:
	GrassTree() = default;
	GrassTree(const GrassTree& other) = default;

	GrassTree& operator=(const GrassTree& other) = default;
	GrassTree(GrassTree&& other) = default;

public:
	void QueryRange(const SimpleMath::Vector2& center, float radius, std::vector<SimpleMath::Vector3>& result) const;
	void SaveToFile(const std::string& filename) const;
	void LoadFromFile(const std::string& filename);
	void BuildTreeFromFile(const std::string& filename);

	size_t GetSize() const;
	std::vector<SimpleMath::Vector3>& GetPoints();
private:
	int ChooseAxis(size_t begin, size_t end);
	int Build(size_t begin, size_t end);
	void QueryRecursive(int nodeIdx, const SimpleMath::Vector2& center, float radius, std::vector<SimpleMath::Vector3>& result) const;

private:
	std::vector<Node> mNodes{};
	std::vector<SimpleMath::Vector3> mPoints{};
	TerrainCollider mTerrainCollider{};
};


class GrassRenderer {
	template<typename T>
	constexpr static T GRASS_INSTANCE_COUNT = static_cast<T>(400 * 2500);

	struct GrassPoint {
		SimpleMath::Vector3 position{};
		float scale{};
		UINT tex{ 0 }; // 하나의 material 안에서의 diffuse 텍스쳐 인덱스
	};

public:
	GrassRenderer() = default;
	GrassRenderer(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator cameraBuffer);
	
	~GrassRenderer() = default;

	GrassRenderer(const GrassRenderer& other) = default;
	GrassRenderer& operator=(const GrassRenderer& other) = default;

	GrassRenderer(GrassRenderer&& other) = default;
	GrassRenderer& operator=(GrassRenderer&& other) = default;
public:
	void SetMaterial(UINT materialIndex);
	void Render(ComPtr<ID3D12GraphicsCommandList6> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material);
private:
	void CreatePipelineState(ComPtr<ID3D12Device10> device);
	void CreateRootSignature(ComPtr<ID3D12Device10> device);
private:
	DefaultBuffer mGrassPosition{}; 

	D3D12_SHADER_BYTECODE mMeshShader{};
	D3D12_SHADER_BYTECODE mPixelShader{};

	UINT mMaterialIndex{ 0 };

	ComPtr<ID3D12RootSignature> mRootSignature{};
	ComPtr<ID3D12PipelineState> mPipelineState{};

	TerrainCollider mTerrainCollider{}; 
	std::vector<SimpleMath::Vector3> mGrass{};
	DefaultBufferCPUIterator mCameraBuffer{}; 
};


