#pragma once 
#include "../Renderer/Resource/Texture.h"
#include "../Utility/Defines.h"
#include "../Utility/DirectXInclude.h"


class ComputeProcessor {
public:
	ComputeProcessor() = default;
	ComputeProcessor(ComPtr<ID3D12Device> device);
	virtual ~ComputeProcessor() = default;

	ComputeProcessor(const ComputeProcessor& other) = default;
	ComputeProcessor& operator=(const ComputeProcessor& other) = default;

	ComputeProcessor(ComputeProcessor&& other) = default;
	ComputeProcessor& operator=(ComputeProcessor&& other) = default;

public:
	virtual void CreateShader(ComPtr<ID3D12Device> device);

	virtual void RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture);
	virtual void Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output = nullptr);

	virtual Texture& GetComputeMap();

private:
	virtual void CreateResource(ComPtr<ID3D12Device> device);
	virtual void CreateHeap(ComPtr<ID3D12Device> device);
	virtual void CreateView(ComPtr<ID3D12Device> device);
	virtual void CompileShader();
	virtual void CreateRootSignature(ComPtr<ID3D12Device> device);
	virtual void CreatePSO(ComPtr<ID3D12Device> device);

protected:
	Texture mComputeMap{}; 

	ComPtr<ID3D12DescriptorHeap> mHeap{ nullptr };
	ComPtr<ID3DBlob> mShaderCode{ nullptr };
	ComPtr<ID3DBlob> mError{ nullptr };
	ComPtr<ID3D12RootSignature> mRootSignature{ nullptr };
	ComPtr<ID3D12PipelineState> mPSO{ nullptr };

};


class HorzBloomProcessor : public ComputeProcessor {
public:
	HorzBloomProcessor() = default;
	HorzBloomProcessor(ComPtr<ID3D12Device> device);
	virtual ~HorzBloomProcessor() = default;

	HorzBloomProcessor(const HorzBloomProcessor& other) = default;
	HorzBloomProcessor& operator=(const HorzBloomProcessor& other) = default;

	HorzBloomProcessor(HorzBloomProcessor&& other) = default;
	HorzBloomProcessor& operator=(HorzBloomProcessor&& other) = default;

public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;


	virtual void RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) override;
	virtual void Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output = nullptr) override;

private:
	virtual void CreateResource(ComPtr<ID3D12Device> device) override;
	virtual void CreateHeap(ComPtr<ID3D12Device> device) override;
	virtual void CreateView(ComPtr<ID3D12Device> device) override;
	virtual void CompileShader() override;
	virtual void CreateRootSignature(ComPtr<ID3D12Device> device) override;
	virtual void CreatePSO(ComPtr<ID3D12Device> device) override;

};

class VertBloomProcessor : public ComputeProcessor {
public:
	VertBloomProcessor() = default;
	VertBloomProcessor(ComPtr<ID3D12Device> device);
	virtual ~VertBloomProcessor() = default;

	VertBloomProcessor(const VertBloomProcessor& other) = default;
	VertBloomProcessor& operator=(const VertBloomProcessor& other) = default;

	VertBloomProcessor(VertBloomProcessor&& other) = default;
	VertBloomProcessor& operator=(VertBloomProcessor&& other) = default;

public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;

	virtual void RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) override;
	virtual void Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output = nullptr) override;

private:
	virtual void CreateResource(ComPtr<ID3D12Device> device) override;
	virtual void CreateHeap(ComPtr<ID3D12Device> device) override;
	virtual void CreateView(ComPtr<ID3D12Device> device) override;
	virtual void CompileShader() override;
	virtual void CreateRootSignature(ComPtr<ID3D12Device> device) override;
	virtual void CreatePSO(ComPtr<ID3D12Device> device) override;

};