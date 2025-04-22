#include "pch.h"
#include "LightingManager.h"

LightingManager::LightingManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mLightCount = 8;
	mLightingBuffer = DefaultBuffer(device, sizeof(Light), mLightCount);
	CreatePointLighting(commandList, SimpleMath::Vector3(0.0f, 12.0f, 4.0f), SimpleMath::Vector4(1.0f, 0.0f, 1.0f, 1.0f));
	CreatePointLighting(commandList, SimpleMath::Vector3(0.0f, 12.0f, 24.0f), SimpleMath::Vector4(0.0f, 0.0f, 1.0f, 1.0f));

	
}

void LightingManager::CreatePointLighting(ComPtr<ID3D12GraphicsCommandList> commandList, SimpleMath::Vector3 position, SimpleMath::Vector4 diffuse) {
	Light pointLight{};
	pointLight.Position = position;
	pointLight.Diffuse = diffuse;

	mLightings.emplace_back(pointLight);
	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

void LightingManager::DeletePointLighting(ComPtr<ID3D12GraphicsCommandList> commandList, int index) {
	mLightings.erase(mLightings.begin() + index);

	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

DefaultBufferGPUIterator LightingManager::GetLightingBuffer() {
	return mLightingBuffer.GPUBegin();
}


