#pragma once 
#include "../Utility/DirectXInclude.h"
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"

class GrassRenderer {
	template<typename T>
	constexpr static T GRASS_INSTANCE_COUNT = static_cast<T>(100'0000);

	struct GrassPoint {
		SimpleMath::Vector2 position{};
		float scale{};
		UINT tex{ 0 }; // 하나의 material 안에서의 diffuse 텍스쳐 인덱스
	};

public:
	GrassRenderer() = default;
	GrassRenderer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator terrainHeader, DefaultBufferGPUIterator terrainData);
	

	~GrassRenderer() = default;


public:
	void SetMaterial(UINT materialIndex); 
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
