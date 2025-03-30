#pragma once 
#include "../Utility/Defines.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Core/Shader.h"


/*


*/
class ParticleManager {
	static constexpr size_t MAX_PARTICLE_COUNT = 10000;
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
	void RenderSO(ComPtr<ID3D12GraphicsCommandList> commandList);
	void RenderGS(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material);
	void PostRender(ComPtr<ID3D12GraphicsCommandList> commandList);
private:
	void BuildRandomBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	
	void SwapBuffer(); 
private:
	std::unique_ptr<GraphicsShaderBase> mParticleSOShader{};
	std::unique_ptr<GraphicsShaderBase> mParticleGSShader{};

	DefaultBuffer mParticleVertexBuffer{};
	D3D12_VERTEX_BUFFER_VIEW mParticleVertexBufferView{};

	DefaultBuffer mParticleSOTargetBuffer{};
	D3D12_STREAM_OUTPUT_BUFFER_VIEW mParticleSOBufferView{};

	DefaultBuffer mEmitParticleBuffer{}; 

	DefaultBuffer mParticleCountBuffer{};

	DefaultBuffer mRandomBuffer{}; 

	ComPtr<ID3D12Resource> mParticleCountReadbackBuffer{};

	UINT32 mParticleCount{ 0 };
};