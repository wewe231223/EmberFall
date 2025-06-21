#pragma once 
#include "../Utility/DirectXInclude.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"
#include "../Renderer/Core/Shader.h"
#include "../MeshLoader/Loader/TerrainLoader.h"

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
	GrassRenderer(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	
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
	D3D12_SHADER_BYTECODE mAmplificationShader{};
	D3D12_SHADER_BYTECODE mPixelShader{};

	UINT mMaterialIndex{ 0 };

	ComPtr<ID3D12RootSignature> mRootSignature{};
	ComPtr<ID3D12PipelineState> mPipelineState{};
};


namespace V2 {
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

		void QueryRange(const SimpleMath::Vector2& center, float radius, std::vector<SimpleMath::Vector3>& result) const;
		void SaveToFile(const std::string& filename) const;
		void LoadFromFile(const std::string& filename);
		void BuildTreeFromFile(const std::string& filename);

		size_t GetSize() const; 
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
	public:
		GrassRenderer() = default; 
		GrassRenderer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator cameraBufferLocation);

		~GrassRenderer() = default;

		GrassRenderer(const GrassRenderer& other) = default;
		GrassRenderer& operator=(const GrassRenderer& other) = default;

		GrassRenderer(GrassRenderer&& other) = default;
		GrassRenderer& operator=(GrassRenderer&& other) = default;

	public:
		void SetMaterial(UINT materialIndex);
		void Render(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material);
	private:
		DefaultBuffer mGrassInstance{}; // StructuredBuffer -	이는 현재 그릴 GrassInstance 의 위치 정보를 담고 있다. 매 프레임 업데이트 된다. 
		
		GrassTree mGrassTree{}; 
		std::vector<SimpleMath::Vector3> mGrass{}; // 매 프레임 컬링을 진행한 결과를 저장하는 장소이다. 

		UINT mMaterialIndex{ 0 };

		std::shared_ptr<GraphicsShaderBase> mShader{};
		DefaultBufferCPUIterator mCameraBufferLocation{};
	};
}