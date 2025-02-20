#include "pch.h"
#include "UploadBuffer.h"
#include "../Utility/Exceptions.h"

UploadBuffer::UploadBuffer(ComPtr<ID3D12Device> device, size_t unitSize, size_t numofElement, void* initialData) {
	mElementSize = unitSize;
	mSize = unitSize * numofElement;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(mSize);

	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mBuffer.GetAddressOf())
	));


	mBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mData));
	if (initialData != nullptr) {
		memcpy(mData, initialData, mSize);
	}

}

UploadBuffer::UploadBuffer(const UploadBuffer& other) {
	if (this != &other) {
		mBuffer = other.mBuffer;
		mData = other.mData;
		mSize = other.mSize;
		mElementSize = other.mElementSize;
	}
}

UploadBuffer& UploadBuffer::operator=(const UploadBuffer& other) {
	if (this != &other) {
		mBuffer = other.mBuffer;
		mData = other.mData;
		mSize = other.mSize;
		mElementSize = other.mElementSize;
	}
	return *this;
}

UploadBuffer::UploadBuffer(UploadBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mData = other.mData;
		mSize = other.mSize;
		mElementSize = other.mElementSize;
	}
}

UploadBuffer& UploadBuffer::operator=(UploadBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mData = other.mData;
		mSize = other.mSize;
		mElementSize = other.mElementSize;
	}
	return *this;
}

BYTE* UploadBuffer::Data() {
    return mData;
}

