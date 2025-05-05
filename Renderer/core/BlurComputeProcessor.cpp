#include "pch.h"
#include "BlurComputeProcessor.h"
#include "../Config/Config.h"
#include "../Utility/Exceptions.h"


BlurComputeProcessor::BlurComputeProcessor(ComPtr<ID3D12Device> device) {
	
	CreateResource(device);
	CreateBlurHeap(device);
	CreateBlurView(device);
	CompileShader();
	CreateRootSignature(device);
	CreatePSO(device);
	
}

void BlurComputeProcessor::DispatchHorzBlur(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> input) {
	
	mHorzBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	mVertBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(mVertBlurMap.GetResource().Get(), input.Get());
	mVertBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandList->SetDescriptorHeaps(1, mBlurHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mBlurRootSignature.Get());
	commandList->SetPipelineState(mHorzBlurPSO.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mBlurHeap->GetGPUDescriptorHandleForHeapStart());
	gpuHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	commandList->SetComputeRootDescriptorTable(0, gpuHandle);
	gpuHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	commandList->SetComputeRootDescriptorTable(1, gpuHandle);
	

	UINT numGroupsX = (UINT)ceilf(Config::WINDOW_WIDTH<UINT> / 256.0f);

	commandList->Dispatch(numGroupsX, Config::WINDOW_HEIGHT<UINT>, 1);


}

void BlurComputeProcessor::DispatchVertBlur(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> output) {
	mVertBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	mHorzBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandList->SetDescriptorHeaps(1, mBlurHeap.GetAddressOf());

	commandList->SetComputeRootSignature(mBlurRootSignature.Get());
	commandList->SetPipelineState(mVertBlurPSO.Get());


	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mBlurHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootDescriptorTable(0, gpuHandle);
	gpuHandle.Offset(3, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	commandList->SetComputeRootDescriptorTable(1, gpuHandle);
	

	UINT numGroupsY = (UINT)ceilf(Config::WINDOW_HEIGHT<UINT> / 256.0f);

	commandList->Dispatch(Config::WINDOW_WIDTH<UINT>, numGroupsY, 1);


	



	mVertBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->CopyResource(output.Get(), mVertBlurMap.GetResource().Get());
	mVertBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	mHorzBlurMap.Transition(commandList, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);



}



Texture& BlurComputeProcessor::GetHorzBlurMap() {
	return mHorzBlurMap;
}

Texture& BlurComputeProcessor::GetVertBlurMap() {
	return mVertBlurMap;
}

void BlurComputeProcessor::CreateResource(ComPtr<ID3D12Device> device) {
	
	mHorzBlurMap = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	mVertBlurMap = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

}

void BlurComputeProcessor::CreateBlurHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC blurDesc{};
	blurDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	blurDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	blurDesc.NumDescriptors = 4; 
	blurDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&blurDesc, IID_PPV_ARGS(mBlurHeap.GetAddressOf()));
	
}

void BlurComputeProcessor::CreateBlurView(ComPtr<ID3D12Device> device) {
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE blurHandle(mBlurHeap->GetCPUDescriptorHandleForHeapStart());
	device->CreateShaderResourceView(mHorzBlurMap.GetResource().Get(), &srvDesc, blurHandle);
	blurHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	device->CreateShaderResourceView(mVertBlurMap.GetResource().Get(), &srvDesc, blurHandle);
	blurHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(mHorzBlurMap.GetResource().Get(), nullptr, &uavDesc, blurHandle);
	blurHandle.Offset(1, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	device->CreateUnorderedAccessView(mVertBlurMap.GetResource().Get(), nullptr, &uavDesc, blurHandle);

}


void BlurComputeProcessor::CompileShader() {
	std::wstring filename = L"Shader/Sources/Blur.hlsl";
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
	HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "HorzBlur_CS", "cs_5_1", flags, 0, mHorzBlurBlob.GetAddressOf(), mError.GetAddressOf());

	CheckHR(hr);
	if (FAILED(hr)) {
		if (mError)
		{
			OutputDebugStringA((char*)mError->GetBufferPointer());
		}
	}

	hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "VertBlur_CS", "cs_5_1", flags, 0, mVertBlurBlob.GetAddressOf(), mError.GetAddressOf());

	CheckHR(hr);
	if (FAILED(hr)) {
		if (mError)
		{
			OutputDebugStringA((char*)mError->GetBufferPointer());
		}
	}
}

void BlurComputeProcessor::CreateRootSignature(ComPtr<ID3D12Device> device) {
	

	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameter[2];

	rootParameter[0].InitAsDescriptorTable(1, &srvTable);
	rootParameter[1].InitAsDescriptorTable(1, &uavTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(2, rootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	
	ComPtr<ID3DBlob> serializedRootSignature{ nullptr };
	ComPtr<ID3DBlob> errorBlob{ nullptr };
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());
	
	device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mBlurRootSignature.GetAddressOf()));

}

void BlurComputeProcessor::CreatePSO(ComPtr<ID3D12Device> device) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC horzBlurPSO = {};
	horzBlurPSO.pRootSignature = mBlurRootSignature.Get();
	horzBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mHorzBlurBlob->GetBufferPointer()),
		mHorzBlurBlob->GetBufferSize()
	};
	horzBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateComputePipelineState(&horzBlurPSO, IID_PPV_ARGS(mHorzBlurPSO.GetAddressOf()));


	D3D12_COMPUTE_PIPELINE_STATE_DESC vertBlurPSO = {};
	vertBlurPSO.pRootSignature = mBlurRootSignature.Get();
	vertBlurPSO.CS =
	{
		reinterpret_cast<BYTE*>(mVertBlurBlob->GetBufferPointer()),
		mVertBlurBlob->GetBufferSize()
	};
	vertBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateComputePipelineState(&vertBlurPSO, IID_PPV_ARGS(mVertBlurPSO.GetAddressOf()));
}

