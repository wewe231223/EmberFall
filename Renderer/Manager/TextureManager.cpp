#include "pch.h"
#include "TextureManager.h"
#include "../Utility/Crash.h"
#include "../Config/Config.h"

TextureManager::TextureManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;

	CheckHR(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mTextureHeap)));

	CrashExp(std::filesystem::exists(TextureManager::IMAGE_DIRECTORY), "Image directory not found.");


	UINT count{ 0 };
	auto pair = std::make_pair(count, Texture(device, commandList, "Resources/Image/DefaultTexture.png"));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ mTextureHeap->GetCPUDescriptorHandleForHeapStart() };
	device->CreateShaderResourceView(pair.second.GetResource().Get(), &srvDesc, handle);

	mTextures["DefaultTexture"] = std::move(pair);

	auto increasement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (const auto& entry : std::filesystem::directory_iterator(TextureManager::IMAGE_DIRECTORY)) {
		auto& path = entry.path();
		auto name = path.stem().string();

		if (mTextures.find(name) != mTextures.end()) {
			continue;
		}


		pair = std::make_pair(++count, Texture(device, commandList, path));

		srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
		handle = mTextureHeap->GetCPUDescriptorHandleForHeapStart();
		handle.Offset(count, increasement);

		device->CreateShaderResourceView(pair.second.GetResource().Get(), &srvDesc, handle);

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
	commandList->SetDescriptorHeaps(1, mTextureHeap.GetAddressOf());
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHeapAddress() {
	return mTextureHeap->GetGPUDescriptorHandleForHeapStart();
}

void MaterialManager::CreateMaterial(const std::string& name, const MaterialConstants& material) {
	mMaterialData.emplace_back(material);
	mMaterials[name] = static_cast<UINT>(mMaterialData.size() - 1);
}

void MaterialManager::CreateMaterial(const std::string& name, UINT diffuseTexture) {
	MaterialConstants material{};
	material.mDiffuseTexture[0] = diffuseTexture;
	CreateMaterial(name, material);
}

void MaterialManager::UploadMaterial(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mMaterialData.shrink_to_fit(); 
	mMaterialBuffer = DefaultBuffer(device, commandList, sizeof(MaterialConstants), mMaterialData.size(), mMaterialData.data());

	::memcpy(mMaterialBuffer.Data(), mMaterialData.data(), mMaterialData.size() * sizeof(MaterialConstants));
	
	mMaterialBuffer.Upload(commandList);
}

MaterialIndex MaterialManager::GetMaterial(const std::string& name) {
	return mMaterials[name];
}

void MaterialManager::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) {
	commandList->SetGraphicsRootShaderResourceView(2, *mMaterialBuffer.GPUBegin());
}

D3D12_GPU_VIRTUAL_ADDRESS MaterialManager::GetMaterialBufferAddress() {
	return *mMaterialBuffer.GPUBegin(); 
}
