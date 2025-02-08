#include "pch.h"
#include "DefaultBuffer.h"

DefaultBuffer::DefaultBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, size_t size, void* initialData) {
	CrashExp(size > 0, "Size must be greater than 0");

	mSize = size;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
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

	CheckHR(mBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mBufferPtr)));

	if (initialData != nullptr) {
		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = initialData;
		subresourceData.RowPitch = size;
		subresourceData.SlicePitch = size;

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &barrier);

		UpdateSubresources(commandList.Get(), mBuffer.Get(), mUploadBuffer.Get(), 0, 0, 1, &subresourceData);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &barrier);
	}
}

DefaultBuffer::DefaultBuffer(ComPtr<ID3D12Device> device, size_t size) {
	CrashExp(size > 0, "Size must be greater than 0");

	mSize = size;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
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

	CheckHR(mBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mBufferPtr)));
}

DefaultBuffer::~DefaultBuffer() {
	if (mBuffer != nullptr) {
		mBuffer->Unmap(0, nullptr);
	}
}

DefaultBuffer::DefaultBuffer(DefaultBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mUploadBuffer = std::move(other.mUploadBuffer);
		mSize = other.mSize;
	}
}

DefaultBuffer& DefaultBuffer::operator=(DefaultBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mUploadBuffer = std::move(other.mUploadBuffer);
		mSize = other.mSize;
	}
	return *this;
}

bool DefaultBuffer::Empty() const {
	return mSize == 0;
}

void DefaultBuffer::Copy(ComPtr<ID3D12GraphicsCommandList> commandList, void* data, std::ptrdiff_t begin, std::ptrdiff_t end) {
	Crash("Not implemented yet");
	//CrashExp(mBufferPtr != nullptr, "DefaultBuffer must be initialized");
	//CrashExp(data != nullptr, "Data must not be nullptr");

	//D3D12_RESOURCE_BARRIER barrier{};

	//if ((begin == 0 && end == 0) || begin > end) {
	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	//	commandList->ResourceBarrier(1, &barrier);

	//	::memcpy(mBufferPtr, data, mSize);
	//	commandList->CopyResource(mBuffer.Get(), mUploadBuffer.Get());

	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandList->ResourceBarrier(1, &barrier);
	//}
	//else {
	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	//	commandList->ResourceBarrier(1, &barrier);

	//	::memcpy(mBufferPtr + begin, data, end - begin);
	//	commandList->CopyBufferRegion(mBuffer.Get(), begin, mUploadBuffer.Get(), begin, end - begin);

	//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandList->ResourceBarrier(1, &barrier);
	//}
}

void DefaultBuffer::AccumulateData(ComPtr<ID3D12GraphicsCommandList> commandList, void* data, size_t size) {
	D3D12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST) };
	commandList->ResourceBarrier(1, &barrier);

	::memcpy(mAccumulationPtr, data, size);

	commandList->CopyResource(mBuffer.Get(), mUploadBuffer.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &barrier);

	mAccumulationPtr += size;
}

void DefaultBuffer::ResetAccumulation() {
	mAccumulationPtr = mBufferPtr;
}
