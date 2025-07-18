#pragma once

#include "../Resource/Texture.h"
#include "../Resource/DefaultBuffer.h"
#include "../Core/Shader.h"
#include "../Config/Config.h"


class VolumetricFogProcessor {
public:
	VolumetricFogProcessor() = default;
	//VolumetricFogProcessor(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~VolumetricFogProcessor() = default;

	VolumetricFogProcessor(const VolumetricFogProcessor& other) = default;
	VolumetricFogProcessor& operator=(const VolumetricFogProcessor& other) = default;

	VolumetricFogProcessor(VolumetricFogProcessor&& other) noexcept = default;
	VolumetricFogProcessor& operator=(VolumetricFogProcessor&& other) noexcept = default;
public:
	void CreateSRVHeap(ComPtr<ID3D12Device> device, Texture& fogVolume);
	void RegisterVelocityMap(ComPtr<ID3D12Device> device, Texture& velocityMap);

	void Render(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS camera);

	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

private:

	ComPtr<ID3D12DescriptorHeap> mVolumetricFogSRVHeap{ nullptr };

	VolumetricFogShader mVolumetricFogShader{};

	std::array<DefaultBuffer, 2> mVolumetricFogPassMesh{};
	std::array<D3D12_VERTEX_BUFFER_VIEW, 2> mVolumetricFogPassMeshView{};

	DefaultBuffer mVolumetricFogPassMeshIndex{};
	D3D12_INDEX_BUFFER_VIEW mVolumetricFogPassMeshIndexView{};

};



