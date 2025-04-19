#pragma once 
#include "../Utility/DirectXInclude.h"
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"

class GrassRenderer {
	template<typename T>
	constexpr static T GRASS_INSTANCE_COUNT = static_cast<T>(2048 * 2500);

	struct GrassPoint {
		SimpleMath::Vector2 position{};
		float scale{};
		UINT tex{ 0 }; // 하나의 material 안에서의 diffuse 텍스쳐 인덱스
	};

public:
	GrassRenderer() = default;
	GrassRenderer(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator terrainHeader, DefaultBufferGPUIterator terrainData);
	
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
	DefaultBufferGPUIterator mTerrainHeader{};
	DefaultBufferGPUIterator mTerrainData{};

	DefaultBuffer mGrassPosition{}; 

	D3D12_SHADER_BYTECODE mMeshShader{};
	D3D12_SHADER_BYTECODE mAmplificationShader{};
	D3D12_SHADER_BYTECODE mPixelShader{};

	UINT mMaterialIndex{ 0 };

	ComPtr<ID3D12RootSignature> mRootSignature{};
	ComPtr<ID3D12PipelineState> mPipelineState{};
};
