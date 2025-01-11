#include "pch.h"
#include "Texture.h"

Texture::Texture() {

}

Texture::Texture(DXGI_FORMAT format, UINT64 width, UINT height, const D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_FLAGS resourceFlag, const D3D12_CLEAR_VALUE& clearValue) {

}

Texture::Texture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& filePath) {

}

Texture::Texture(Texture&& other) noexcept {
	mResource = std::move(other.mResource);
	
	if (nullptr != mUploadbuffer) {
		mUploadbuffer = std::move(other.mUploadbuffer);
	}
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		mResource = std::move(other.mResource);
		if (nullptr != mUploadbuffer) {
			mUploadbuffer = std::move(other.mUploadbuffer);
		}
	}

	return *this;
}

ComPtr<ID3D12Resource> Texture::Get() {
	return mResource;
}

ID3D12Resource** Texture::GetAddressOf() {
	return mResource.GetAddressOf();
}
