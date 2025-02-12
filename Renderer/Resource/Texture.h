#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Texture.h
// 2025.01.11 김승범   - 이미지 정보를 GPU 에 저장하는 Texture 클래스를 정의함. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <filesystem>
#include "../Utility/DirectXInclude.h"

class Texture {
public:
	Texture();
	Texture(ComPtr<ID3D12Device> device, DXGI_FORMAT format, UINT64 width, UINT height, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_FLAGS resourceFlag);
	Texture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& filePath);

	Texture(const Texture&);
	Texture& operator=(const Texture&);

	Texture(Texture&&) noexcept;
	Texture& operator=(Texture&&) noexcept;
public:
	ComPtr<ID3D12Resource> GetResource();
	ID3D12Resource** GetAddressOf();
private:
	void LoadDefaultTexture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
private:
	ComPtr<ID3D12Resource> mResource{ nullptr };
	ComPtr<ID3D12Resource> mUploadbuffer{ nullptr };
};