#include "pch.h"
#include "Texture.h"
#include "../External/Include/DirectXTK12/DDSTextureLoader.h"
#include "../External/Include/DirectXTK12/WICTextureLoader.h"
#include "../Utility/Exceptions.h"
#include "../EditorInterface/Console/Console.h"

Texture::Texture() {

}

Texture::Texture(ComPtr<ID3D12Device> device, DXGI_FORMAT format, UINT64 width, UINT height, D3D12_HEAP_FLAGS heapFlag, D3D12_RESOURCE_FLAGS resourceFlag) {
	D3D12_RESOURCE_DESC desc{ CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, resourceFlag) };
	D3D12_HEAP_PROPERTIES heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };

	D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

	if (resourceFlag & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
		resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}
	else if (resourceFlag & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
		resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}

	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		heapFlag,
		&desc,
		resourceState,
		nullptr,
		IID_PPV_ARGS(mResource.GetAddressOf())
	));
}



Texture::Texture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& filePath) {
	if (false == std::filesystem::exists(filePath)) {
		Console.Log("{} 파일을 찾지 못했습니다. 기본 텍스쳐를 로딩합니다.", LogType::Warning, filePath.string().c_str());
		Texture::LoadDefaultTexture(device, commandList);
	}
	else {
		std::wstring extension = filePath.extension().wstring();
		std::unique_ptr<uint8_t[]> data{ nullptr };

		if (extension == L".dds" or extension == L".DDS") {
			std::vector<D3D12_SUBRESOURCE_DATA> subresources{};
			CheckHR(DirectX::LoadDDSTextureFromFile(device.Get(), filePath.c_str(), mResource.GetAddressOf(), data, subresources));

			auto uploadBufferSize = GetRequiredIntermediateSize(mResource.Get(), 0, static_cast<UINT>(subresources.size()));

			CD3DX12_RESOURCE_DESC desc{ CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize) };
			CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD };

			CheckHR(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(mUploadbuffer.GetAddressOf())
			));

			CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ) };
			commandList->ResourceBarrier(1, &barrier);

			UpdateSubresources(commandList.Get(), mResource.Get(), mUploadbuffer.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_GENERIC_READ);
			commandList->ResourceBarrier(1, &barrier);
		}
		else {
			D3D12_SUBRESOURCE_DATA subresource{};
			CheckHR(DirectX::LoadWICTextureFromFile(device.Get(), filePath.c_str(), mResource.GetAddressOf(), data, subresource));

			auto uploadBufferSize = GetRequiredIntermediateSize(mResource.Get(), 0, 1);

			CD3DX12_RESOURCE_DESC desc{ CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize) };
			CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD };

			CheckHR(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(mUploadbuffer.GetAddressOf())
			));

			CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ) };
			commandList->ResourceBarrier(1, &barrier);

			UpdateSubresources(commandList.Get(), mResource.Get(), mUploadbuffer.Get(), 0, 0, 1, &subresource);

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_GENERIC_READ);
			commandList->ResourceBarrier(1, &barrier);
		}

		Console.Log("{} 파일을 성공적으로 로딩했습니다.", LogType::Info, filePath.string().c_str());
	}


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

void Texture::LoadDefaultTexture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::wstring filePath = L"Resources/Textures/DefaultTexture.png";
	std::unique_ptr<uint8_t[]> data{ nullptr };

	D3D12_SUBRESOURCE_DATA subresource{};
	CheckHR(DirectX::LoadWICTextureFromFile(device.Get(), filePath.c_str(), mResource.GetAddressOf(), data, subresource));

	auto uploadBufferSize = GetRequiredIntermediateSize(mResource.Get(), 0, 1);

	CD3DX12_RESOURCE_DESC desc{ CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize) };
	CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD };

	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mUploadbuffer.GetAddressOf())
	));

	CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ) };
	commandList->ResourceBarrier(1, &barrier);

	UpdateSubresources(commandList.Get(), mResource.Get(), mUploadbuffer.Get(), 0, 0, 1, &subresource);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &barrier);
}
