#include "pch.h"
#include "ComputeProcessor.h"
#include "../Config/Config.h"
#include "../Utility/Exceptions.h"


ComputeProcessor::ComputeProcessor(ComPtr<ID3D12Device> device) {

}

void ComputeProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	CreateResource(device);
	CreateHeap(device);
	CreateView(device);
	CompileShader();
	CreateRootSignature(device);
	CreatePSO(device);
}

void ComputeProcessor::RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) {
	
}

void ComputeProcessor::Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output) {
	
}

Texture& ComputeProcessor::GetComputeMap() {
	return mComputeMap;
}

void ComputeProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	  
}

void ComputeProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	
}

void ComputeProcessor::CreateView(ComPtr<ID3D12Device> device) {

}

void ComputeProcessor::CompileShader() {
	
}

void ComputeProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {

}

void ComputeProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
	
}





HorzBloomProcessor::HorzBloomProcessor(ComPtr<ID3D12Device> device) {

}

void HorzBloomProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	ComputeProcessor::CreateShader(device);
}

void HorzBloomProcessor::RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
	blurHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(texture.GetResource().Get(), &srvDesc, blurHandle);
}

void HorzBloomProcessor::Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output) {
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(mComputeMap.GetResource().Get(), input->GetResource().Get());
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetDescriptorHeaps(1, mHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPSO.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->SetComputeRootDescriptorTable(0, gpuHandle); //uav

	gpuHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	commandList->SetComputeRootDescriptorTable(1, gpuHandle); //srv
	

	UINT numGroupsX = static_cast<UINT>(std::ceilf(Config::WINDOW_WIDTH<UINT> / 256.0f));

	commandList->Dispatch(numGroupsX, Config::WINDOW_HEIGHT<UINT>, 1);
}

void HorzBloomProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	mComputeMap = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void HorzBloomProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC blurDesc{};
	blurDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	blurDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	blurDesc.NumDescriptors = 2;
	blurDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&blurDesc, IID_PPV_ARGS(mHeap.GetAddressOf()));
}

void HorzBloomProcessor::CreateView(ComPtr<ID3D12Device> device) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mComputeMap.GetResource().Get(), nullptr, &uavDesc, blurHandle);
	
}

void HorzBloomProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/Bloom.hlsl";
	std::filesystem::path filePath(filename);

	if (std::filesystem::exists(filePath)) {
		std::wstring message = L"file exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
	else {
		std::wstring message = L"file not exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
#ifdef _DEBUG
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "HorzBlur_CS", "cs_5_1", flags, 0, mShaderCode.GetAddressOf(), mError.GetAddressOf());

	CheckHR(hr);
	if (FAILED(hr)) {
		if (mError) {
			OutputDebugStringA((char*)mError->GetBufferPointer());
		}
	}
}

void HorzBloomProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {

	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE emissiveTable;
	emissiveTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	

	CD3DX12_ROOT_PARAMETER rootParameter[2];

	rootParameter[0].InitAsDescriptorTable(1, &uavTable);
	rootParameter[1].InitAsDescriptorTable(1, &emissiveTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(2, rootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void HorzBloomProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC horzBlurPSO = {};
	horzBlurPSO.pRootSignature = mRootSignature.Get();
	horzBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaderCode->GetBufferPointer()),
		mShaderCode->GetBufferSize()
	};
	horzBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateComputePipelineState(&horzBlurPSO, IID_PPV_ARGS(mPSO.GetAddressOf()));
}



VertBloomProcessor::VertBloomProcessor(ComPtr<ID3D12Device> device) {

}

void VertBloomProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	ComputeProcessor::CreateShader(device);
}

void VertBloomProcessor::RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
	blurHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	device->CreateShaderResourceView(texture.GetResource().Get(), &srvDesc, blurHandle);

}

void VertBloomProcessor::Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output) {
	input->Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(mComputeMap.GetResource().Get(), output->GetResource().Get());
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetDescriptorHeaps(1, mHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPSO.Get());


	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootDescriptorTable(0, gpuHandle); //uav
	gpuHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	commandList->SetComputeRootDescriptorTable(1, gpuHandle); //srv

	UINT numGroupsY = static_cast<UINT>(ceilf(Config::WINDOW_HEIGHT<UINT> / 256.0f));

	commandList->Dispatch(Config::WINDOW_WIDTH<UINT>, numGroupsY, 1);

	output->Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->CopyResource(output->GetResource().Get(), mComputeMap.GetResource().Get());
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	input->Transition(commandList, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
}


void VertBloomProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	mComputeMap = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

}

void VertBloomProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC blurDesc{};
	blurDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	blurDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	blurDesc.NumDescriptors = 2;
	blurDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&blurDesc, IID_PPV_ARGS(mHeap.GetAddressOf()));
}

void VertBloomProcessor::CreateView(ComPtr<ID3D12Device> device) {

	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());



	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mComputeMap.GetResource().Get(), nullptr, &uavDesc, blurHandle);
}

void VertBloomProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/Bloom.hlsl";
	
#ifdef _DEBUG
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "VertBlur_CS", "cs_5_1", flags, 0, mShaderCode.GetAddressOf(), mError.GetAddressOf());

	CheckHR(hr);
	if (FAILED(hr)) {
		if (mError) {
			OutputDebugStringA((char*)mError->GetBufferPointer());
		}
	}
}

void VertBloomProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {
	
	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	
	CD3DX12_ROOT_PARAMETER rootParameter[2];

	rootParameter[0].InitAsDescriptorTable(1, &uavTable);
	rootParameter[1].InitAsDescriptorTable(1, &srvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(2, rootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void VertBloomProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC vertBlurPSO = {};
	vertBlurPSO.pRootSignature = mRootSignature.Get();
	vertBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaderCode->GetBufferPointer()),
		mShaderCode->GetBufferSize()
	};
	vertBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateComputePipelineState(&vertBlurPSO, IID_PPV_ARGS(mPSO.GetAddressOf()));
}




