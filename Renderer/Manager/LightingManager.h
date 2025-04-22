#pragma once
#include "../Core/ShadowRenderer.h"

struct Light
{
    
    SimpleMath::Vector4 Diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };          // Diffuse 색상
    SimpleMath::Vector4 Ambient{ 0.3f, 0.3f, 0.3f, 1.0f };          // Ambient 색상
    SimpleMath::Vector4 Specular{ 1.0f, 1.0f, 1.0f, 1.0f };         // Specular 색상
    SimpleMath::Vector3 Position;                                   // 조명 위치 
    SimpleMath::Vector3 Direction;                                  // 조명 방향 
    SimpleMath::Vector3 Attenuation{ 1.0f, 0.09f, 0.032f };         // 감쇠 계수
    float Range{6.0f};                                              // 조명 범위
};

class LightingManager {
public:
	LightingManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~LightingManager() = default;

	LightingManager(const LightingManager& other) = delete;
	LightingManager& operator=(const LightingManager& other) = delete;

	LightingManager(LightingManager&& other) = default;
	LightingManager& operator=(LightingManager&& other) = default;

public:
    void CreatePointLighting(ComPtr<ID3D12GraphicsCommandList> commandList, SimpleMath::Vector3 position, SimpleMath::Vector4 diffuse);
    void DeletePointLighting(ComPtr<ID3D12GraphicsCommandList> commandList, int index);

    DefaultBufferGPUIterator GetLightingBuffer();
private:

private:
    int mLightCount{};

	std::vector<Light> mLightings; // TODO : 자료구조 바꾸기
	DefaultBuffer mLightingBuffer{};
};
