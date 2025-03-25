#pragma once 
#include "../Renderer/Resource/Texture.h"

class ShadowRenderer {
	
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
	void TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList);

private:
	ComPtr<ID3D12DescriptorHeap> mShadowDSVHeap{};

	Texture mShadowMap{}; 
};