#pragma once 
#include "../Utility/Defines.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Core/Shader.h"
#include "../Renderer/Resource/Particle.h"

class ParticleManager {
	static constexpr size_t MAX_PARTICLE_COUNT = 10'0000;
	static constexpr size_t EMIT_PARTICLE_COUNT = 100;
public:
	ParticleManager() = default;
	~ParticleManager() = default;

	ParticleManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

	ParticleManager(const ParticleManager&) = default;
	ParticleManager& operator=(const ParticleManager&) = default;

	ParticleManager(ParticleManager&&) = default;
	ParticleManager& operator=(ParticleManager&&) = default;

public:
	void SetTerrain(DefaultBufferGPUIterator terrainHeader, DefaultBufferGPUIterator terrainData); 

	Particle CreateEmitParticle(ComPtr<ID3D12GraphicsCommandList> commandList, ParticleVertex& newParticle); 

	void RenderSO(ComPtr<ID3D12GraphicsCommandList> commandList);
	void RenderGS(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material);
	void PostRender();

	void ValidateParticle(); 
private:
	void BuildRandomBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	
	void SwapBuffer(); 
private:
	std::unique_ptr<GraphicsShaderBase> mParticleSOShader{};
	std::unique_ptr<GraphicsShaderBase> mParticleGSShader{};

	DefaultBuffer mParticleVertexBuffer{};
	DefaultBufferCPUIterator mNewParticleUploadLoc{};
	UINT32 mNewParticleCount{ 0 };
	D3D12_VERTEX_BUFFER_VIEW mParticleVertexBufferView{};

	DefaultBuffer mParticleSOTargetBuffer{};
	D3D12_STREAM_OUTPUT_BUFFER_VIEW mParticleSOBufferView{};

	DefaultBuffer mEmitParticleBuffer{}; 

	DefaultBuffer mParticleCountBuffer{};
	DefaultBuffer mRandomBuffer{}; 

	DefaultBufferGPUIterator mTerrainHeaderBuffer{};
	DefaultBufferGPUIterator mTerrainDataBuffer{};

	ComPtr<ID3D12Resource> mParticleCountReadbackBuffer{};

	UINT32 mParticleCount{ 0 };

	std::array<EmitParticleContext, EMIT_PARTICLE_COUNT> mEmitParticleContexts{};
	UINT mNextEmitParticleIndex{ 0 };
};