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
	static constexpr T SHADOWMAPSIZE = static_cast<T>(2000);

	static constexpr float NEAROFFSET = 30.0f;  // 조명 투영행렬의 근,원평면에 약간의 여유 공간을 추가할때 사용.
	static constexpr float FAROFFSET = 0.0f;

	static constexpr float PROJECTIONOFFSET = 5.0f; // 조명 투영공간의 크기를 조정하기 위한 오프셋값.

	static constexpr std::array<float, Config::SHADOWMAP_COUNT<int>> SHADOWMAPOFFSET = { 20.0f, 90.0f };
public:
	ShadowRenderer() = default;
	ShadowRenderer(ComPtr<ID3D12Device> device, DefaultBufferCPUIterator mainCameraBufferLoc);
	
	~ShadowRenderer() = default;

	ShadowRenderer(const ShadowRenderer&) = default;
	ShadowRenderer& operator=(const ShadowRenderer&) = default;

	ShadowRenderer(ShadowRenderer&&) = default;
	ShadowRenderer& operator=(ShadowRenderer&&) = default;

public:
	// 그림자 맵 만들기 
	void SetShadowDSVRTV(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList,int index);

	void Update(); 
	void Upload(ComPtr<ID3D12GraphicsCommandList> commandList);
	DefaultBufferGPUIterator GetShadowCameraBuffer(int index);

	bool IsInShadowFrustum(int index, Collider& other);
	bool IsInShadowFrustum(int index, DirectX::BoundingBox& other);

	void TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	Texture& GetShadowMap(int index);
	std::array<Texture, Config::SHADOWMAP_COUNT<int>>& GetShadowMapArray();

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

	std::array<DefaultBuffer, Config::SHADOWMAP_COUNT<int>> mShadowCameraBuffer{};

	CameraConstants mShadowCamera{};

	std::vector<DirectX::BoundingOrientedBox> mWorldBox{};
	std::array<SimpleMath::Matrix, Config::SHADOWMAP_COUNT<int>> mLightMatrix{};

	std::array<Texture, Config::SHADOWMAP_COUNT<int>> mShadowMap{};
	Texture mDepthStencilMap{};

	DefaultBufferCPUIterator mMainCamerabuffeerLocation{}; 
};