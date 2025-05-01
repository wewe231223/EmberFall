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
    CrashExp(std::filesystem::exists(LOAD_DIRECTORY), "Load directory not found.");

    // Initial default texture
    mCount = 0;
    {
        std::filesystem::path defaultPath = std::filesystem::path(IMAGE_DIRECTORY) / "DefaultTexture.png";
        auto pair = std::make_pair(mCount, Texture(device, commandList, defaultPath));
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = pair.second.GetResource()->GetDesc().MipLevels;

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mTextureHeap->GetCPUDescriptorHandleForHeapStart());
        device->CreateShaderResourceView(pair.second.GetResource().Get(), &srvDesc, handle);

        mTextures["DefaultTexture"] = std::move(pair);
    }

    UINT increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // Load only files in Load directory
    for (const auto& entry : std::filesystem::directory_iterator(LOAD_DIRECTORY)) {
        const auto& path = entry.path();
        const std::string name = path.stem().string();
        if (mTextures.find(name) != mTextures.end() || !std::filesystem::is_regular_file(path))
            continue;

        auto pair = std::make_pair(++mCount, Texture(device, commandList, path));
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = pair.second.GetResource()->GetDesc().MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mTextureHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(mCount, increment);
        device->CreateShaderResourceView(pair.second.GetResource().Get(), &srvDesc, handle);

        mTextures[name] = std::move(pair);
    }
}

UINT TextureManager::GetTexture(const std::string& name) {
    auto it = mTextures.find(name);
    if (it == mTextures.end()) {
        return mTextures["DefaultTexture"].first;
    }
    return it->second.first;
}

void TextureManager::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) {
    commandList->SetDescriptorHeaps(1, mTextureHeap.GetAddressOf());
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHeapAddress() {
    return mTextureHeap->GetGPUDescriptorHandleForHeapStart();
}

void TextureManager::LoadAllImages(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
    UINT increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    for (const auto& entry : std::filesystem::directory_iterator(IMAGE_DIRECTORY)) {
        const auto& path = entry.path();
        const std::string name = path.stem().string();
        if (path == std::filesystem::path(LOAD_DIRECTORY) ||
            mTextures.find(name) != mTextures.end() ||
            !std::filesystem::is_regular_file(path))
            continue;

        auto pair = std::make_pair(++mCount, Texture(device, commandList, path));
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = pair.second.GetResource()->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = pair.second.GetResource()->GetDesc().MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mTextureHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(mCount, increment);
        device->CreateShaderResourceView(pair.second.GetResource().Get(), &srvDesc, handle);

        mTextures[name] = std::move(pair);
    }
}


MaterialManager::MaterialManager(ComPtr<ID3D12Device> device) {
	mMaterialBuffer = DefaultBuffer(device, sizeof(MaterialConstants), MAX_MATERIAL_COUNTS);
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

	if (mMaterialData.size() == 0) {
        MaterialConstants dummy{};
		mMaterialData.emplace_back(dummy);
	}

	::memcpy(*mMaterialBuffer.CPUBegin(), mMaterialData.data(), mMaterialData.size() * sizeof(MaterialConstants));
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
