#pragma once 
#include "../Renderer/Resource/Texture.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"
#include "../Utility/DirectXInclude.h"
#include "../Game/GameObject/Collider.h"


class ShadowRenderer {
	static constexpr SimpleMath::Vector3 LIGHTDIRECTION{ 1.f, -3.f, -1.f };

	template<typename T>
	static constexpr T SHADOWMAPSIZE = static_cast<T>(1000);

	//그림자 맵에 담을 프러스텀의 원평면 까지의 거리
	static constexpr float FRUSTUMLENGTH = 10.0f;
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
	void SetShadowDSVRTV(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Update(DefaultBufferCPUIterator worldCameraBuffer); 
	void Upload(ComPtr<ID3D12GraphicsCommandList> commandList);
	bool ShadowMapCulling(Collider& other);

	DefaultBufferGPUIterator GetShadowCameraBuffer(); 

	Texture& GetShadowMap();

	template<typename T>
	static constexpr T GetShadowMapSize() { return SHADOWMAPSIZE<T>; }
private:
	std::array<SimpleMath::Vector3, 8> ComputeFrustumCorners(SimpleMath::Matrix worldCameraVPInv);
	void ComputeOrientedBoundingBox(SimpleMath::Matrix cameraProjInv);
	void StablizeShadowMatrix(SimpleMath::Matrix& shadowCameraVP);

private:
	ComPtr<ID3D12DescriptorHeap> mShadowRTVHeap{};
	ComPtr<ID3D12DescriptorHeap> mShadowDSVHeap{};

	DefaultBuffer mShadowCameraBuffer{};

	CameraConstants mShadowCamera{};

	DirectX::BoundingOrientedBox mWorldBox{};
	

	Texture mShadowMap{}; 
	Texture mDepthStencilMap{};


};