#include "pch.h"
#include "DefaultBuffer.h"

DefaultBuffer::DefaultBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, size_t unitSize, size_t numofElement, void* initialData) {
	CrashExp(unitSize * numofElement > 0, "Size must be greater than 0");

	mElementSize = unitSize;
	mSize = unitSize * numofElement;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(mSize);
	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBuffer)
	));

	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mUploadBuffer)
	));

	CheckHR(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mBufferPtr)));

	if (initialData != nullptr) {
		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = initialData;
		subresourceData.RowPitch = mSize;
		subresourceData.SlicePitch = mSize;

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &barrier);

		UpdateSubresources(commandList.Get(), mBuffer.Get(), mUploadBuffer.Get(), 0, 0, 1, &subresourceData);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &barrier);
	}
}

DefaultBuffer::DefaultBuffer(ComPtr<ID3D12Device> device, size_t unitSize, size_t numofElement){
	CrashExp(unitSize * numofElement > 0, "Size must be greater than 0");

	mElementSize = unitSize;
	mSize = unitSize * numofElement;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(mSize);
	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBuffer)
	));

	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CheckHR(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mUploadBuffer)
	));

	CheckHR(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mBufferPtr)));
}

DefaultBuffer::~DefaultBuffer() {
	if (mBuffer != nullptr) {
		mUploadBuffer->Unmap(0, nullptr);
	}
}

DefaultBuffer::DefaultBuffer(DefaultBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mUploadBuffer = std::move(other.mUploadBuffer);
		mElementSize = other.mElementSize;
		mSize = other.mSize;
	}
}

DefaultBuffer& DefaultBuffer::operator=(DefaultBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mUploadBuffer = std::move(other.mUploadBuffer);
		mElementSize = other.mElementSize;
		mSize = other.mSize;
	}
	return *this;
}

bool DefaultBuffer::Empty() const {
	return mElementSize == 0;
}

BYTE* DefaultBuffer::Data() {
	return mBufferPtr;
}

void DefaultBuffer::Upload(ComPtr<ID3D12GraphicsCommandList> commandList) {
	D3D12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST) };
	commandList->ResourceBarrier(1, &barrier);

	commandList->CopyResource(mBuffer.Get(), mUploadBuffer.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &barrier);
}

