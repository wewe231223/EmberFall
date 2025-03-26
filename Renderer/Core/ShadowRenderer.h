#pragma once 
#include "../Renderer/Resource/Texture.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"
#include "../Utility/DirectXInclude.h"

class ShadowRenderer {
	static constexpr SimpleMath::Vector3 LIGHTDIRECTION{ 1.f, -1.f, 1.f };

	template<typename T>
	static constexpr T SHADOWMAPSIZE = static_cast<T>(2000);
public:
	ShadowRenderer() = default;
	ShadowRenderer(ComPtr<ID3D12Device> device);
	
	~ShadowRenderer() = default;

	ShadowRenderer(const ShadowRenderer&) = default;
	ShadowRenderer& operator=(const ShadowRenderer&) = default;

	ShadowRenderer(ShadowRenderer&&) = default;
	ShadowRenderer& operator=(ShadowRenderer&&) = default;

public:
	// 그림자 맵 만들기 
	void SetShadowDSV(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Update(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator worldCameraBuffer); 
	void TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList);

	DefaultBufferGPUIterator GetShadowCameraBuffer(); 

	Texture& GetShadowMap();
private:
	std::array<SimpleMath::Vector3, 8> ComputeFrustumCorners(SimpleMath::Matrix worldCameraVPInv);
	void StablizeShadowMatrix(SimpleMath::Matrix& shadowCameraVP);

private:
	ComPtr<ID3D12DescriptorHeap> mShadowDSVHeap{};

	DefaultBuffer mShadowCameraBuffer{};

	CameraConstants mShadowCamera{};

	Texture mShadowMap{}; 
};