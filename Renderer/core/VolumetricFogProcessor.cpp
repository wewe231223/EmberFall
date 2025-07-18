#include "pch.h"
#include "VolumetricFogProcessor.h"


void VolumetricFogProcessor::CreateSRVHeap(ComPtr<ID3D12Device> device, Texture& fogVolume) {
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	CheckHR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mVolumetricFogSRVHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mVolumetricFogSRVHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture3D.MostDetailedMip = 0;    
	srvDesc.Texture3D.MipLevels = -1;  
	srvDesc.Texture3D.ResourceMinLODClamp = 0;
	device->CreateShaderResourceView(fogVolume.GetResource().Get(), &srvDesc, handle);
}

void VolumetricFogProcessor::RegisterVelocityMap(ComPtr<ID3D12Device> device, Texture& velocityMap) {
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};

	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = 1;
	viewDesc.Format = velocityMap.GetResource()->GetDesc().Format;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ mVolumetricFogSRVHeap->GetCPUDescriptorHandleForHeapStart() };
	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle.Offset(1, descriptorSize);


	device->CreateShaderResourceView(velocityMap.GetResource().Get(), &viewDesc, handle);
}

void VolumetricFogProcessor::Render(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	mVolumetricFogShader.SetGPassShader(commandList);

	commandList->SetDescriptorHeaps(1, mVolumetricFogSRVHeap.GetAddressOf());

	commandList->IASetVertexBuffers(0, 2, mVolumetricFogPassMeshView.data());
	commandList->IASetIndexBuffer(&mVolumetricFogPassMeshIndexView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	CD3DX12_GPU_DESCRIPTOR_HANDLE handle{ mVolumetricFogSRVHeap->GetGPUDescriptorHandleForHeapStart() };
	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	commandList->SetGraphicsRootDescriptorTable(0, handle);

	handle.Offset(1, descriptorSize);
	commandList->SetGraphicsRootDescriptorTable(1, handle);

	commandList->SetGraphicsRootConstantBufferView(2, camera);


	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void VolumetricFogProcessor::BuildShader(ComPtr<ID3D12Device> device) {
	mVolumetricFogShader.CreateShader(device);

}

void VolumetricFogProcessor::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
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

	mVolumetricFogPassMesh[0] = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector3), _countof(positions), positions);
	mVolumetricFogPassMesh[1] = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector2), _countof(texCoords), texCoords);

	mVolumetricFogPassMeshIndex = DefaultBuffer(device, commandList, sizeof(UINT), _countof(indices), indices);

	mVolumetricFogPassMeshView[0].BufferLocation = *mVolumetricFogPassMesh[0].GPUBegin();
	mVolumetricFogPassMeshView[0].SizeInBytes = sizeof(SimpleMath::Vector3) * _countof(positions);
	mVolumetricFogPassMeshView[0].StrideInBytes = sizeof(SimpleMath::Vector3);

	mVolumetricFogPassMeshView[1].BufferLocation = *mVolumetricFogPassMesh[1].GPUBegin();
	mVolumetricFogPassMeshView[1].SizeInBytes = sizeof(SimpleMath::Vector2) * _countof(texCoords);
	mVolumetricFogPassMeshView[1].StrideInBytes = sizeof(SimpleMath::Vector2);

	mVolumetricFogPassMeshIndexView.BufferLocation = *mVolumetricFogPassMeshIndex.GPUBegin();
	mVolumetricFogPassMeshIndexView.SizeInBytes = sizeof(UINT) * _countof(indices);
	mVolumetricFogPassMeshIndexView.Format = DXGI_FORMAT_R32_UINT;
}

