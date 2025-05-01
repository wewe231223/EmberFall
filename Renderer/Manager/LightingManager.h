#pragma once
#include "../Core/ShadowRenderer.h"

enum class LightType : UINT {
	None = 0, 
	Directional = 1,
	Point = 2,
	Spot = 3,
};

struct Light {
	LightType mType{ LightType::None }; 
    SimpleMath::Vector4 Diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };          // Diffuse 색상
    SimpleMath::Vector4 Ambient{ 0.3f, 0.3f, 0.3f, 1.0f };          // Ambient 색상
    SimpleMath::Vector4 Specular{ 1.0f, 1.0f, 1.0f, 1.0f };         // Specular 색상
    SimpleMath::Vector3 Position;                                   // 조명 위치 
    SimpleMath::Vector3 Direction;                                  // 조명 방향 
    SimpleMath::Vector3 Attenuation{ 1.f, 0.045f, 0.0075f };        // 감쇠 계수
	float InnerAngle{ 30.f };
	float OuterAngle{ 45.f };                                       // Spot 조명 각도
    float Range{2.0f};                                              // 조명 범위
};

class LightingManager {
	static constexpr size_t MAX_LIGHT_COUNT = 8;
public:
	LightingManager() = default;
	LightingManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~LightingManager() = default;

	LightingManager(const LightingManager& other) = default;
	LightingManager& operator=(const LightingManager& other) = default;

	LightingManager(LightingManager&& other) = default;
	LightingManager& operator=(LightingManager&& other) = default;

public:
	void ClearLight(ComPtr<ID3D12GraphicsCommandList> commandList); 

	// **  DEPRECATED ** // 
    void CreatePointLight(ComPtr<ID3D12GraphicsCommandList> commandList, const SimpleMath::Vector3& position, const SimpleMath::Vector4& diffuse);
	void CreateDirectionalLight(ComPtr<ID3D12GraphicsCommandList> commandList, const SimpleMath::Vector3& direction, const SimpleMath::Vector4& diffuse);

	Light& GetLight(int index); 
	void UpdateLight(ComPtr<ID3D12GraphicsCommandList> commandList);

    void DeleteLight(ComPtr<ID3D12GraphicsCommandList> commandList, int index);

    DefaultBufferGPUIterator GetLightingBuffer();
private:

private:
    std::vector<Light> mLightings{}; // TODO : 자료구조 바꾸기
	DefaultBuffer mLightingBuffer{};
};
