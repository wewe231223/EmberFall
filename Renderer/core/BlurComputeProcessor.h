#pragma once 
#include "../Renderer/Resource/Texture.h"
#include "../Utility/Defines.h"
#include "../Utility/DirectXInclude.h"

class BlurComputeProcessor {
public:
	BlurComputeProcessor() = default;
	BlurComputeProcessor(ComPtr<ID3D12Device> device);
	~BlurComputeProcessor() = default;

	BlurComputeProcessor(const BlurComputeProcessor& other) = default;
	BlurComputeProcessor& operator=(const BlurComputeProcessor& other) = default;

	BlurComputeProcessor(BlurComputeProcessor&& other) = default;
	BlurComputeProcessor& operator=(BlurComputeProcessor&& other) = default;



public:
	void DispatchHorzBlur(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> input);
	void DispatchVertBlur(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> output);

	void RegisteremissiveView(ComPtr<ID3D12Device> device, Texture& EmissiveGbuffer);

	Texture& GetHorzBlurMap();
	Texture& GetVertBlurMap();



private:
	void CreateResource(ComPtr<ID3D12Device> device);
	void CreateBlurHeap(ComPtr<ID3D12Device> device);
	void CreateBlurView(ComPtr<ID3D12Device> device);
	void CompileShader();
	void CreateRootSignature(ComPtr<ID3D12Device> device);
	void CreatePSO(ComPtr<ID3D12Device> device);
private:
	
	Texture mHorzBlurMap{}; 
	Texture mVertBlurMap{};

	ComPtr<ID3D12DescriptorHeap> mBlurHeap{ nullptr };

	ComPtr<ID3DBlob> mHorzBlurBlob{ nullptr };
	ComPtr<ID3DBlob> mVertBlurBlob{ nullptr };
	ComPtr<ID3DBlob> mError{ nullptr };

	ComPtr<ID3D12RootSignature> mBlurRootSignature{ nullptr };

	ComPtr<ID3D12PipelineState> mHorzBlurPSO{ nullptr };
	ComPtr<ID3D12PipelineState> mVertBlurPSO{ nullptr };
};