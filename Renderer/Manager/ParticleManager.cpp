#include "pch.h"
#include "ParticleManager.h"
#include <random>
#include "../Utility/Exceptions.h"

ParticleManager::ParticleManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mParticleVertexBuffer = DefaultBuffer(device, sizeof(ParticleVertex), MAX_PARTICLE_COUNT);
	mParticleVertexBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	mParticleSOTargetBuffer = DefaultBuffer(device, sizeof(ParticleVertex), MAX_PARTICLE_COUNT);
	mParticleSOTargetBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_STREAM_OUT);

	mEmitParticleBuffer = DefaultBuffer(device, sizeof(DirectX::XMFLOAT3), EMIT_PARTICLE_COUNT);
	mEmitParticleBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	mParticleCountBuffer = DefaultBuffer(device, sizeof(UINT64), 1);
	mParticleCountBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_STREAM_OUT);

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
	mParticleCountBuffer.Upload(commandList, D3D12_RESOURCE_STATE_STREAM_OUT);

	mParticleSOShader->SetGPassShader(commandList);

	mParticleSOBufferView.BufferLocation = *mParticleSOTargetBuffer.GPUBegin(); 
	mParticleSOBufferView.SizeInBytes = sizeof(ParticleVertex) * MAX_PARTICLE_COUNT;
	mParticleSOBufferView.BufferFilledSizeLocation = *mParticleCountBuffer.GPUBegin();

	mParticleVertexBufferView.BufferLocation = *mParticleVertexBuffer.GPUBegin();
	mParticleVertexBufferView.SizeInBytes = sizeof(ParticleVertex) * MAX_PARTICLE_COUNT;
	mParticleVertexBufferView.StrideInBytes = sizeof(ParticleVertex);

	commandList->SOSetTargets(0, 1, &mParticleSOBufferView);
	commandList->IASetVertexBuffers(0, 1, &mParticleVertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Resource Set, Draw Call 



	mParticleCountBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->CopyResource(mParticleCountReadbackBuffer.Get(), mParticleCountBuffer.Get());
	mParticleCountBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_STREAM_OUT);
}

void ParticleManager::RenderGS(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material) {
	mParticleVertexBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_STREAM_OUT);
	mParticleSOTargetBuffer.TransitionState(commandList, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	ParticleManager::SwapBuffer(); 

	mParticleGSShader->SetGPassShader(commandList);

	mParticleVertexBufferView.BufferLocation = *mParticleVertexBuffer.GPUBegin();
	mParticleVertexBufferView.SizeInBytes = sizeof(ParticleVertex) * MAX_PARTICLE_COUNT;
	mParticleVertexBufferView.StrideInBytes = sizeof(ParticleVertex);

	commandList->SOSetTargets(0, 1, nullptr);
	commandList->IASetVertexBuffers(0, 1, &mParticleVertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);


	// Resource Set, Draw Call 
}

void ParticleManager::PostRender(ComPtr<ID3D12GraphicsCommandList> commandList) {
	UINT64* data = nullptr;
	CheckHR(mParticleCountReadbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data)));
	mParticleCount = static_cast<UINT32>((*data) / sizeof(ParticleVertex));
	mParticleCountReadbackBuffer->Unmap(0, nullptr);
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

void ParticleManager::SwapBuffer() {
	DefaultBuffer temp{};
	temp = std::move(mParticleVertexBuffer);
	mParticleVertexBuffer = std::move(mParticleSOTargetBuffer);
	mParticleSOTargetBuffer = std::move(temp);
}
