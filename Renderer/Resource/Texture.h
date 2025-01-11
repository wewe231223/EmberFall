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
	
	Texture(DXGI_FORMAT format, UINT64 width, UINT height, const D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_FLAGS resourceFlag,const D3D12_CLEAR_VALUE& clearValue);
	Texture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& filePath);

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) noexcept;
	Texture& operator=(Texture&&) noexcept;
public:
	ComPtr<ID3D12Resource> Get();
	ID3D12Resource** GetAddressOf();


private:
	ComPtr<ID3D12Resource> mResource{ nullptr };

	ComPtr<ID3D12Resource> mUploadbuffer{ nullptr };
};