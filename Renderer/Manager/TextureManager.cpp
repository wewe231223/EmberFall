#include "pch.h"
#include "TextureManager.h"
#include "../Utility/Crash.h"

TextureManager::TextureManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = MAX_TEXTURE_COUNT<UINT>;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;

	CheckHR(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mTextureHeap)));

	CrashExp(std::filesystem::exists(TextureManager::IMAGE_DIRECTORY), "Image directory not found.");


	UINT count{ 0 };
	auto pair = std::make_pair(count++, Texture(device, commandList, "Resources/Image/Default.png"));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ mTextureHeap->GetCPUDescriptorHandleForHeapStart() };
	device->CreateShaderResourceView(pair.second.GetResource().Get(), &srvDesc, handle);

	mTextures["Default"] = std::move(pair);

	auto increasement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (const auto& entry : std::filesystem::directory_iterator(TextureManager::IMAGE_DIRECTORY)) {
		auto& path = entry.path();
		auto name = path.stem().string();
		
		pair = std::make_pair(count++, Texture(device, commandList, path));

		srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
		handle = mTextureHeap->GetCPUDescriptorHandleForHeapStart();
		handle.Offset(count, increasement);

		mTextures[name] = std::move(pair);
	}
}

UINT TextureManager::GetTexture(const std::string& name) {
	auto result = mTextures.find(name);
	if (result == mTextures.end()) {
		return mTextures["Default"].first;
	}
	return result->second.first;
}

void TextureManager::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) {
	commandList->SetGraphicsRootDescriptorTable(0, mTextureHeap->GetGPUDescriptorHandleForHeapStart());
}

void MaterialManager::CreateMaterial(const std::string& name, const MaterialConstants& material) {

}

void MaterialManager::CreateMaterial(const std::string& name, const MaterialConstants& material, UINT diffuseTexture) {

}

void MaterialManager::UploadMaterial(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {

}

void MaterialManager::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) {

}
