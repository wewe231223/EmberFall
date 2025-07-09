#include "pch.h"
#include "MotionBlurProcessor.h"

MotionBlurProcessor::MotionBlurProcessor(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mRTResource = Texture(device, DXGI_FORMAT_R8G8B8A8_UNORM, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_NONE);

}

void MotionBlurProcessor::CreateSRVHeap(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	CheckHR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mMotionBlurSRVHeap)));


	D3D12_SHADER_RESOURCE_VIEW_DESC veiwDesc{};
	veiwDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	veiwDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	veiwDesc.Texture2D.MipLevels = 1;
	veiwDesc.Format = mRTResource.GetResource()->GetDesc().Format;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ mMotionBlurSRVHeap->GetCPUDescriptorHandleForHeapStart() };

	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	
	device->CreateShaderResourceView(mRTResource.GetResource().Get(), &veiwDesc, handle);
		
	
}

void MotionBlurProcessor::RegisterVelocityMap(ComPtr<ID3D12Device> device, Texture& velocityMap) {
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = 1;
	viewDesc.Format = velocityMap.GetResource()->GetDesc().Format;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ mMotionBlurSRVHeap->GetCPUDescriptorHandleForHeapStart() };
	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, descriptorSize);


	device->CreateShaderResourceView(velocityMap.GetResource().Get(), &viewDesc, handle);
}

void MotionBlurProcessor::Render(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mMotionBlurShader.SetGPassShader(commandList);

	commandList->SetDescriptorHeaps(1, mMotionBlurSRVHeap.GetAddressOf());

	commandList->IASetVertexBuffers(0, 2, mMotionBlurPassMeshView.data());
	commandList->IASetIndexBuffer(&mMotionBlurPassMeshIndexView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	CD3DX12_GPU_DESCRIPTOR_HANDLE handle{ mMotionBlurSRVHeap->GetGPUDescriptorHandleForHeapStart() };
	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	commandList->SetGraphicsRootDescriptorTable(0, handle);

	handle.Offset(1, descriptorSize);
	commandList->SetGraphicsRootDescriptorTable(1, handle);
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void MotionBlurProcessor::BuildShader(ComPtr<ID3D12Device> device) {
	mMotionBlurShader.CreateShader(device);

}

void MotionBlurProcessor::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	SimpleMath::Vector3 positions[] = {
		{ -1.0f,  1.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f },
		{ -1.0f, -1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f }
	};

	SimpleMath::Vector2 texCoords[] = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f }
	};

	UINT indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	mMotionBlurPassMesh[0] = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector3), _countof(positions), positions);
	mMotionBlurPassMesh[1] = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector2), _countof(texCoords), texCoords);

	mMotionBlurPassMeshIndex = DefaultBuffer(device, commandList, sizeof(UINT), _countof(indices), indices);

	mMotionBlurPassMeshView[0].BufferLocation = *mMotionBlurPassMesh[0].GPUBegin();
	mMotionBlurPassMeshView[0].SizeInBytes = sizeof(SimpleMath::Vector3) * _countof(positions);
	mMotionBlurPassMeshView[0].StrideInBytes = sizeof(SimpleMath::Vector3);

	mMotionBlurPassMeshView[1].BufferLocation = *mMotionBlurPassMesh[1].GPUBegin();
	mMotionBlurPassMeshView[1].SizeInBytes = sizeof(SimpleMath::Vector2) * _countof(texCoords);
	mMotionBlurPassMeshView[1].StrideInBytes = sizeof(SimpleMath::Vector2);

	mMotionBlurPassMeshIndexView.BufferLocation = *mMotionBlurPassMeshIndex.GPUBegin();
	mMotionBlurPassMeshIndexView.SizeInBytes = sizeof(UINT) * _countof(indices);
	mMotionBlurPassMeshIndexView.Format = DXGI_FORMAT_R32_UINT;
}
