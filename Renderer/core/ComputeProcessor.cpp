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

void ComputeProcessor::DispatchFogProcessor(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS light, D3D12_GPU_VIRTUAL_ADDRESS camera) {

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

	/*if (std::filesystem::exists(filePath)) {
		std::wstring message = L"file exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
	else {
		std::wstring message = L"file not exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}*/
#ifdef _DEBUG
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "HorzBloom_CS", "cs_5_1", flags, 0, mShaderCode.GetAddressOf(), mError.GetAddressOf());

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

	UINT numGroupsY = static_cast<UINT>(std::ceilf(Config::WINDOW_HEIGHT<UINT> / 256.0f));

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

	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "VertBloom_CS", "cs_5_1", flags, 0, mShaderCode.GetAddressOf(), mError.GetAddressOf());

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





HorzBlurProcessor::HorzBlurProcessor(ComPtr<ID3D12Device> device) {

}

void HorzBlurProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	ComputeProcessor::CreateShader(device);
}

void HorzBlurProcessor::RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());
	blurHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(texture.GetResource().Get(), &srvDesc, blurHandle);
}

void HorzBlurProcessor::Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output) {
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(mComputeMap.GetResource().Get(), input->GetResource().Get());
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->SetDescriptorHeaps(1, mHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPSO.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->SetComputeRootDescriptorTable(0, gpuHandle); //uav

	


	UINT numGroupsX = static_cast<UINT>((Config::WINDOW_WIDTH<UINT>  + 255.0f) / 256.0f);

	commandList->Dispatch(numGroupsX, Config::WINDOW_HEIGHT<UINT>, 1);
}

void HorzBlurProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	mComputeMap = Texture(device, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

}

void HorzBlurProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC blurDesc{};
	blurDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	blurDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	blurDesc.NumDescriptors = 2;
	blurDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&blurDesc, IID_PPV_ARGS(mHeap.GetAddressOf()));
}

void HorzBlurProcessor::CreateView(ComPtr<ID3D12Device> device) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mComputeMap.GetResource().Get(), nullptr, &uavDesc, blurHandle);
}

void HorzBlurProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/Blur.hlsl";
	std::filesystem::path filePath(filename);

	/*if (std::filesystem::exists(filePath)) {
		std::wstring message = L"file exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
	else {
		std::wstring message = L"file not exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}*/
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

void HorzBlurProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {
	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	


	CD3DX12_ROOT_PARAMETER rootParameter[1];

	rootParameter[0].InitAsDescriptorTable(1, &uavTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1, rootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void HorzBlurProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
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






VertBlurProcessor::VertBlurProcessor(ComPtr<ID3D12Device> device){

}

void VertBlurProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	ComputeProcessor::CreateShader(device);

}

void VertBlurProcessor::RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) {

}

void VertBlurProcessor::Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output) {
	input->Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(mComputeMap.GetResource().Get(), input->GetResource().Get());
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	input->Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

	commandList->SetDescriptorHeaps(1, mHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPSO.Get());


	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootDescriptorTable(0, gpuHandle); //uav
	

	UINT numGroupsY = static_cast<UINT>((Config::WINDOW_HEIGHT<UINT> + 255.0f) / 256.0f);

	commandList->Dispatch(Config::WINDOW_WIDTH<UINT>, numGroupsY, 1);

	output->Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->CopyResource(output->GetResource().Get(), mComputeMap.GetResource().Get());
	mComputeMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	input->Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

}

void VertBlurProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	mComputeMap = Texture(device, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

}

void VertBlurProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC blurDesc{};
	blurDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	blurDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	blurDesc.NumDescriptors = 2;
	blurDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&blurDesc, IID_PPV_ARGS(mHeap.GetAddressOf()));
}

void VertBlurProcessor::CreateView(ComPtr<ID3D12Device> device) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mComputeMap.GetResource().Get(), nullptr, &uavDesc, blurHandle);
}

void VertBlurProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/Blur.hlsl";
	std::filesystem::path filePath(filename);

	/*if (std::filesystem::exists(filePath)) {
		std::wstring message = L"file exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
	else {
		std::wstring message = L"file not exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}*/
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

void VertBlurProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {
	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);




	CD3DX12_ROOT_PARAMETER rootParameter[1];

	rootParameter[0].InitAsDescriptorTable(1, &uavTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1, rootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void VertBlurProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
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




FogComputeProcessor::FogComputeProcessor(ComPtr<ID3D12Device> device) {

}

void FogComputeProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	ComputeProcessor::CreateShader(device);

}



void FogComputeProcessor::RegisterShadowMap(ComPtr<ID3D12Device> device, Texture& texture1 , Texture& texture2) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(texture1.GetResource().Get(), &srvDesc, handle);

	

	handle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(texture2.GetResource().Get(), &srvDesc, handle);
}

void FogComputeProcessor::DispatchFogProcessor(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS light, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	commandList->SetDescriptorHeaps(1, mHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPSO.Get());


	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootDescriptorTable(0, gpuHandle); //uav
	gpuHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	commandList->SetComputeRootDescriptorTable(1, gpuHandle); //srv

	commandList->SetComputeRootConstantBufferView(2, camera);
	commandList->SetComputeRootShaderResourceView(3, light);


	UINT numGroupsX = static_cast<UINT>((Config::WINDOW_WIDTH<UINT> / mResourceOffset +7.0f) / 8.0f);
	UINT numGroupsY = static_cast<UINT>((Config::WINDOW_HEIGHT<UINT> / mResourceOffset +7.0f) / 8.0f);
	UINT numGroupsZ = static_cast<UINT>((64 +7.0f) / 8.0f);

	commandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}



void FogComputeProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	mComputeMap = Texture(device, DXGI_FORMAT_R16G16B16A16_FLOAT, Config::WINDOW_WIDTH<UINT64> / mResourceOffset, Config::WINDOW_HEIGHT<UINT> / mResourceOffset, 64,
		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

}

void FogComputeProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC fogComputeDesc{};
	fogComputeDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	fogComputeDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	fogComputeDesc.NumDescriptors = 3;
	fogComputeDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&fogComputeDesc, IID_PPV_ARGS(mHeap.GetAddressOf()));
}

void FogComputeProcessor::CreateView(ComPtr<ID3D12Device> device) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.FirstWSlice = 0;  
	uavDesc.Texture3D.WSize = -1;
	device->CreateUnorderedAccessView(mComputeMap.GetResource().Get(), nullptr, &uavDesc, handle);
}

void FogComputeProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/FogComputeProcessor.hlsl";
	std::filesystem::path filePath(filename);

	/*if (std::filesystem::exists(filePath)) {
		std::wstring message = L"file exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
	else {
		std::wstring message = L"file not exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}*/
#ifdef _DEBUG
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "FogComputeProcessor_CS", "cs_5_1", flags, 0,
		mShaderCode.GetAddressOf(), mError.GetAddressOf());

	CheckHR(hr);
	if (FAILED(hr)) {
		if (mError) {
			OutputDebugStringA((char*)mError->GetBufferPointer());
		}
	}
}

void FogComputeProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 7> staticSamplers{};

	staticSamplers[0] = { 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_GREATER_EQUAL };
	staticSamplers[1] = { 1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_GREATER_EQUAL };
	staticSamplers[2] = { 2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_GREATER_EQUAL };
	staticSamplers[3] = { 3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_GREATER_EQUAL };
	staticSamplers[4] = { 4, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 16, D3D12_COMPARISON_FUNC_GREATER_EQUAL };
	staticSamplers[5] = { 5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_GREATER_EQUAL };
	staticSamplers[6] = { 6, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0.0f, 16, D3D12_COMPARISON_FUNC_GREATER_EQUAL };

	
	
	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable1;
	srvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

	

	CD3DX12_ROOT_PARAMETER rootParameter[4];

	rootParameter[0].InitAsDescriptorTable(1, &uavTable);
	rootParameter[1].InitAsDescriptorTable(1, &srvTable1);

	rootParameter[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

	rootParameter[3].InitAsShaderResourceView(2, 0, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameter, 7, staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void FogComputeProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC fogComputePSO = {};
	fogComputePSO.pRootSignature = mRootSignature.Get();
	fogComputePSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaderCode->GetBufferPointer()),
		mShaderCode->GetBufferSize()
	};
	fogComputePSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateComputePipelineState(&fogComputePSO, IID_PPV_ARGS(mPSO.GetAddressOf()));
}






FogAccumulateProcessor::FogAccumulateProcessor(ComPtr<ID3D12Device> device) {

}

void FogAccumulateProcessor::CreateShader(ComPtr<ID3D12Device> device) {
	ComputeProcessor::CreateShader(device);

}

void FogAccumulateProcessor::RegisterTexture(ComPtr<ID3D12Device> device, Texture& texture) {


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	device->CreateShaderResourceView(texture.GetResource().Get(), &srvDesc, handle);
	
}

void FogAccumulateProcessor::Dispatch(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Texture* input, Texture* output) {
	commandList->SetDescriptorHeaps(1, mHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPSO.Get());


	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootDescriptorTable(0, gpuHandle); //uav
	
	gpuHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	commandList->SetComputeRootDescriptorTable(1, gpuHandle); //srv

	UINT numGroupsX = static_cast<UINT>((Config::WINDOW_WIDTH<UINT> / mResourceOffset + 7.0f) / 8.0f);
	UINT numGroupsY = static_cast<UINT>((Config::WINDOW_HEIGHT<UINT> / mResourceOffset + 7.0f) / 8.0f);
	UINT numGroupsZ = 64;

	commandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
}


void FogAccumulateProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	mComputeMap = Texture(device, DXGI_FORMAT_R16G16B16A16_FLOAT, Config::WINDOW_WIDTH<UINT64> / mResourceOffset, Config::WINDOW_HEIGHT<UINT> / mResourceOffset, 64,
		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void FogAccumulateProcessor::CreateHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC fogAccumulateDesc{};
	fogAccumulateDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	fogAccumulateDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	fogAccumulateDesc.NumDescriptors = 2;
	fogAccumulateDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&fogAccumulateDesc, IID_PPV_ARGS(mHeap.GetAddressOf()));
}

void FogAccumulateProcessor::CreateView(ComPtr<ID3D12Device> device) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.WSize = mComputeMap.GetResource()->GetDesc().DepthOrArraySize;
	device->CreateUnorderedAccessView(mComputeMap.GetResource().Get(), nullptr, &uavDesc, handle);
}

void FogAccumulateProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/FogAccumulateProcessor.hlsl";
	std::filesystem::path filePath(filename);

	/*if (std::filesystem::exists(filePath)) {
		std::wstring message = L"file exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}
	else {
		std::wstring message = L"file not exists :" + filePath.wstring() + L"\n";
		OutputDebugStringW(message.c_str());
	}*/
#ifdef _DEBUG
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "FogAccumulateProcessor_CS", "cs_5_1", flags, 0,
		mShaderCode.GetAddressOf(), mError.GetAddressOf());

	CheckHR(hr);
	if (FAILED(hr)) {
		if (mError) {
			OutputDebugStringA((char*)mError->GetBufferPointer());
		}
	}
}

void FogAccumulateProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {
	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameter[2];

	rootParameter[0].InitAsDescriptorTable(1, &uavTable);
	rootParameter[1].InitAsDescriptorTable(1, &srvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(2, rootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void FogAccumulateProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC fogAccumulatePSO = {};
	fogAccumulatePSO.pRootSignature = mRootSignature.Get();
	fogAccumulatePSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaderCode->GetBufferPointer()),
		mShaderCode->GetBufferSize()
	};
	fogAccumulatePSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateComputePipelineState(&fogAccumulatePSO, IID_PPV_ARGS(mPSO.GetAddressOf()));
}
