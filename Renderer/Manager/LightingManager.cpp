#include "pch.h"
#include "LightingManager.h"

LightingManager::LightingManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mLightingBuffer = DefaultBuffer(device, sizeof(Light), MAX_LIGHT_COUNT);
	mLightings.resize(8); 
}

void LightingManager::ClearLight(ComPtr<ID3D12GraphicsCommandList> commandList) {
	// mLightings.clear();

	std::memset(*mLightingBuffer.CPUBegin(), 0, sizeof(Light) * MAX_LIGHT_COUNT);

	mLightingBuffer.Upload(commandList);
}

void LightingManager::CreatePointLight(ComPtr<ID3D12GraphicsCommandList> commandList, const SimpleMath::Vector3& position, const SimpleMath::Vector4& diffuse) {
	Light pointLight{};
	pointLight.mType = LightType::Point;
	pointLight.Position = position;
	pointLight.Diffuse = diffuse;
	pointLight.Specular = { 0.f,0.f,0.f,0.f };

	mLightings.emplace_back(pointLight);
	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

void LightingManager::CreateDirectionalLight(ComPtr<ID3D12GraphicsCommandList> commandList, const SimpleMath::Vector3& direction, const SimpleMath::Vector4& diffuse) {
	Light directionalLight{};
	directionalLight.Direction = direction;
	directionalLight.Diffuse = diffuse;
	directionalLight.mType = LightType::Directional;

	mLightings.emplace_back(directionalLight);
	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

Light& LightingManager::GetLight(int index) {
	return mLightings[index];
}

void LightingManager::UpdateLight(ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());
	mLightingBuffer.Upload(commandList);
}

void LightingManager::DeleteLight(ComPtr<ID3D12GraphicsCommandList> commandList, int index) {
	mLightings.erase(mLightings.begin() + index);

	std::memcpy(*mLightingBuffer.CPUBegin(), mLightings.data(), sizeof(Light) * mLightings.size());

	mLightingBuffer.Upload(commandList);
}

DefaultBufferGPUIterator LightingManager::GetLightingBuffer() {
	return mLightingBuffer.GPUBegin();
}


