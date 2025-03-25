#include "pch.h"
#include "DefferedRenderer.h"
#include "../Config/Config.h"
#include <filesystem>

DefferedRenderer::DefferedRenderer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = Config::GBUFFER_COUNT<UINT>;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	CheckHR(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mGBufferSRVHeap)));

	DefferedRenderer::BuildShader(device);
	DefferedRenderer::BuildMesh(device, commandList);
}

void DefferedRenderer::RegisterGBufferTexture(ComPtr<ID3D12Device> device, const std::span<Texture>& textures) {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ mGBufferSRVHeap->GetCPUDescriptorHandleForHeapStart() };

	auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto& texture : textures) {
		desc.Format = texture.GetResource()->GetDesc().Format;
		device->CreateShaderResourceView(texture.GetResource().Get(), &desc, handle);
		handle.Offset(1, descriptorSize);
	}

}

void DefferedRenderer::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mDefferedShader.SetGPassShader(commandList);

	commandList->SetDescriptorHeaps(1, mGBufferSRVHeap.GetAddressOf());

	commandList->IASetVertexBuffers(0, 2, mLightPassMeshView.data());
	commandList->IASetIndexBuffer(&mLightPassMeshIndexView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootDescriptorTable(0, mGBufferSRVHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void DefferedRenderer::BuildShader(ComPtr<ID3D12Device> device) {
	mDefferedShader.CreateShader(device);
}

void DefferedRenderer::BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
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

	mLightPassMesh[0] = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector3), _countof(positions), positions);
	mLightPassMesh[1] = DefaultBuffer(device, commandList, sizeof(SimpleMath::Vector2), _countof(texCoords), texCoords);

	mLightPassMeshIndex = DefaultBuffer(device, commandList, sizeof(UINT), _countof(indices), indices);

	mLightPassMeshView[0].BufferLocation = *mLightPassMesh[0].GPUBegin(); 
	mLightPassMeshView[0].SizeInBytes = sizeof(SimpleMath::Vector3) * _countof(positions);
	mLightPassMeshView[0].StrideInBytes = sizeof(SimpleMath::Vector3);

	mLightPassMeshView[1].BufferLocation = *mLightPassMesh[1].GPUBegin();
	mLightPassMeshView[1].SizeInBytes = sizeof(SimpleMath::Vector2) * _countof(texCoords);
	mLightPassMeshView[1].StrideInBytes = sizeof(SimpleMath::Vector2);

	mLightPassMeshIndexView.BufferLocation = *mLightPassMeshIndex.GPUBegin();
	mLightPassMeshIndexView.SizeInBytes = sizeof(UINT) * _countof(indices);
	mLightPassMeshIndexView.Format = DXGI_FORMAT_R32_UINT;

}
