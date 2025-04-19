#pragma once 
#include <array>
#include <span>
#include "../Resource/DefaultBuffer.h"
#include "../Resource/Texture.h"
#include "../Core/Shader.h"

class DefferedRenderer {
public:
	DefferedRenderer() = default;
	DefferedRenderer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~DefferedRenderer() = default;

	DefferedRenderer(const DefferedRenderer& other) = default;
	DefferedRenderer& operator=(const DefferedRenderer& other) = default;

	DefferedRenderer(DefferedRenderer&& other) noexcept = default;
	DefferedRenderer& operator=(DefferedRenderer&& other) noexcept = default;
public:
	void RegisterGBufferTexture(ComPtr<ID3D12Device> device, const std::span<Texture>& textures);
	void RegisterShadowMap(ComPtr<ID3D12Device> device, std::array<Texture, 3>& shadowMap);
	void Render(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator shadowCameraBuffer, DefaultBufferGPUIterator lightingBuffer);
private:
	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
private:
	ComPtr<ID3D12DescriptorHeap> mGBufferSRVHeap{ nullptr };
	
	DefferedShader mDefferedShader{};

	std::array<DefaultBuffer, 2> mLightPassMesh{};
	std::array<D3D12_VERTEX_BUFFER_VIEW, 2> mLightPassMeshView{};

	DefaultBuffer mLightPassMeshIndex{};
	D3D12_INDEX_BUFFER_VIEW mLightPassMeshIndexView{};
};