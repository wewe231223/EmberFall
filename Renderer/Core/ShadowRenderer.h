#pragma once 
#include "../Renderer/Resource/Texture.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Utility/Defines.h"
#include "../Utility/DirectXInclude.h"
#include "../Game/GameObject/Collider.h"
#include "../Game/Scene/Camera.h"


class ShadowRenderer {
	static constexpr SimpleMath::Vector3 LIGHTDIRECTION{ 1.f, -3.f, -1.f };

	template<typename T>
	static constexpr T SHADOWMAPSIZE = static_cast<T>(1000);

	//그림자 맵에 담을 프러스텀의 원평면 까지의 거리
	static constexpr float FRUSTUMLENGTH = 10.0f;

	static constexpr std::array<float, 3> SHADOWMAPOFFSET = { 10.0f, 30.0f, 100.0f };
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
	void SetShadowDSVRTV(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList,int index);
	void Update(DefaultBufferCPUIterator worldCameraBuffer); 
	void Upload(ComPtr<ID3D12GraphicsCommandList> commandList);
	bool ShadowMapCulling(int index, Collider& other);
	void TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	DefaultBufferGPUIterator GetShadowCameraBuffer(int index);

	Texture& GetShadowMap(int index);
	std::array<Texture, 3>& GetShadowMapArray();

	template<typename T>
	static constexpr T GetShadowMapSize() { return SHADOWMAPSIZE<T>; }
private:
	std::array<SimpleMath::Vector3, 8> ComputeFrustumCorners(SimpleMath::Matrix worldCameraVPInv);
	SimpleMath::Matrix ComputeLightViewMatrix(CameraParameter cameraParam, SimpleMath::Matrix invView, float nearZ, float farZ);
	void ComputeOrientedBoundingBox(SimpleMath::Matrix cameraProjInv);
	void StablizeShadowMatrix(SimpleMath::Matrix& shadowCameraVP);

private:
	ComPtr<ID3D12DescriptorHeap> mShadowRTVHeap{};
	ComPtr<ID3D12DescriptorHeap> mShadowDSVHeap{};

	std::array<DefaultBuffer, 3> mShadowCameraBuffer{};

	CameraConstants mShadowCamera{};

	std::vector<DirectX::BoundingOrientedBox> mWorldBox{};
	std::array<SimpleMath::Matrix, 3> mLightMatrix{};

	std::array<Texture,3> mShadowMap{};
	Texture mDepthStencilMap{};


};