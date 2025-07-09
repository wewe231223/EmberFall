#pragma once

#include "../Resource/DefaultBuffer.h"
#include "../Resource/Texture.h"
#include "../Core/Shader.h"
#include "../Config/Config.h"

class MotionBlurProcessor {
public:
	MotionBlurProcessor() = default;
	MotionBlurProcessor(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~MotionBlurProcessor() = default;

	MotionBlurProcessor(const MotionBlurProcessor& other) = default;
	MotionBlurProcessor& operator=(const MotionBlurProcessor& other) = default;

	MotionBlurProcessor(MotionBlurProcessor&& other) noexcept = default;
	MotionBlurProcessor& operator=(MotionBlurProcessor&& other) noexcept = default;
public:
	void CreateSRVHeap(ComPtr<ID3D12Device> device);

	void RegisterVelocityMap(ComPtr<ID3D12Device> device, Texture& velocityMap);
	void Render(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

	void BuildShader(ComPtr<ID3D12Device> device);
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

	Texture& GetRTResource() { return mRTResource; };
private:
	Texture mRTResource{ };

	ComPtr<ID3D12DescriptorHeap> mMotionBlurSRVHeap{ nullptr };

	MotionBlurShader mMotionBlurShader{};

	std::array<DefaultBuffer, 2> mMotionBlurPassMesh{};
	std::array<D3D12_VERTEX_BUFFER_VIEW, 2> mMotionBlurPassMeshView{};

	DefaultBuffer mMotionBlurPassMeshIndex{};
	D3D12_INDEX_BUFFER_VIEW mMotionBlurPassMeshIndexView{};

};

