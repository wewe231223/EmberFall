#include "pch.h"
#include "ParticleManager.h"
#include <random>
#include "../Utility/Exceptions.h"

ParticleManager::ParticleManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mParticleVertexBuffer = DefaultBuffer(device, sizeof(ParticleVertex), MAX_PARTICLE_COUNT);
	mParticleSOTargetBuffer = DefaultBuffer(device, sizeof(ParticleVertex), MAX_PARTICLE_COUNT);

	mParticleCountBuffer = DefaultBuffer(device, sizeof(UINT64), 1);

	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64));
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mParticleCountReadbackBuffer)
	));
	
	ParticleManager::BuildRandomBuffer(device, commandList);
}

void ParticleManager::RenderSO(ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::memset(*mParticleCountBuffer.CPUBegin(), 0, sizeof(UINT64));
	mParticleCountBuffer.Upload(commandList);

	mParticleSOShader->SetGPassShader(commandList);

	mParticleSOBufferView.BufferLocation = *mParticleSOTargetBuffer.GPUBegin(); 
	mParticleSOBufferView.SizeInBytes = sizeof(ParticleVertex) * MAX_PARTICLE_COUNT;
	mParticleSOBufferView.BufferFilledSizeLocation = *mParticleCountBuffer.GPUBegin();

	mParticleVertexBufferView.BufferLocation = *mParticleVertexBuffer.GPUBegin();
	mParticleVertexBufferView.SizeInBytes = sizeof(ParticleVertex) * MAX_PARTICLE_COUNT;
	mParticleVertexBufferView.StrideInBytes = sizeof(ParticleVertex);




}

void ParticleManager::BuildRandomBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::random_device rd;
	std::default_random_engine engine(rd());
	std::uniform_real_distribution<float> urf(0.0f, 1.0f);

	std::vector<float> randomData(64 * 64);

	for (auto& data : randomData) {
		data = urf(engine);
	}

	mRandomBuffer = DefaultBuffer(device, commandList, sizeof(float), 64 * 64, randomData.data());
}
