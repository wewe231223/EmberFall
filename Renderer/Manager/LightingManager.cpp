#include "pch.h"
#include "LightingManager.h"

LightingManager::LightingManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, int playerNum) {
	mLightCount = playerNum * 2;
	mLightings.resize(mLightCount);
	mLightingBuffer = DefaultBuffer(device, sizeof(Light), mLightCount);


}

void LightingManager::CreatePointLighting(ComPtr<ID3D12GraphicsCommandList> commandList, SimpleMath::Vector3 position) {
	Light pointLight{};
	pointLight.Position = position;


	mLightings.emplace_back(pointLight);
	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

void LightingManager::DeletePointLighting(ComPtr<ID3D12GraphicsCommandList> commandList, int index) {
	mLightings.erase(mLightings.begin() + index);

	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

DefaultBufferGPUIterator LightingManager::GetLightingBuffer()
{
	return mLightingBuffer.GPUBegin();
}


