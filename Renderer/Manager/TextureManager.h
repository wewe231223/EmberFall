#pragma once 
#include <filesystem>
#include <unordered_map>
#include "../Utility/DirectXInclude.h"
#include "../Utility/Defines.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Resource/Texture.h"

#ifdef max
#undef max
#endif
class TextureManager {
	static constexpr const char* IMAGE_DIRECTORY = "Resources/Image";
public:
	TextureManager() = default;
	TextureManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~TextureManager() = default;

	TextureManager(const TextureManager& other) = delete;
	TextureManager& operator=(const TextureManager& other) = delete;

	TextureManager(TextureManager&& other) = default;
	TextureManager& operator=(TextureManager&& other) = default;
public:
	UINT GetTexture(const std::string& name);

	void Bind(ComPtr<ID3D12GraphicsCommandList> commandList); 

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHeapAddress();
private:
	ComPtr<ID3D12DescriptorHeap> mTextureHeap{ nullptr };
	std::unordered_map<std::string, std::pair<UINT,Texture>> mTextures{};
};

struct MaterialConstants {
	SimpleMath::Color mDiffuseColor{};
	SimpleMath::Color mSpecularColor{};
	SimpleMath::Color mEmissiveColor{};

	UINT mDiffuseTexture[3]{};
	UINT mSpecularTexture[3]{};
	UINT mMetalicTexture[3]{};
	UINT mEmissiveTexture[3]{};
	UINT mNormalTexture[3]{};
	UINT mAlphaTexture[3]{};
};

class MaterialManager {
public:
	MaterialManager() = default;
	~MaterialManager() = default;

	MaterialManager(const MaterialManager& other) = delete;
	MaterialManager& operator=(const MaterialManager& other) = delete;

	MaterialManager(MaterialManager&& other) = default;
	MaterialManager& operator=(MaterialManager&& other) = default;
public:
	void CreateMaterial(const std::string& name, const MaterialConstants& material);
	void CreateMaterial(const std::string& name, UINT diffuseTexture);

	void UploadMaterial(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList); 

	MaterialIndex GetMaterial(const std::string& name);

	void Bind(ComPtr<ID3D12GraphicsCommandList> commandList);

	D3D12_GPU_VIRTUAL_ADDRESS GetMaterialBufferAddress();
private:
	DefaultBuffer mMaterialBuffer{};
	std::vector<MaterialConstants> mMaterialData{};
	std::unordered_map<std::string, MaterialIndex> mMaterials{};
};