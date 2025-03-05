#include "pch.h"
#include "DefaultBuffer.h"
#include "../Utility/Defines.h"

DefaultBuffer::DefaultBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, size_t unitSize, size_t numofElement, const void* initialData, bool constant) {
	CrashExp(unitSize * numofElement > 0, "Size must be greater than 0");

	mElementSize = unitSize;
	mSize = unitSize * numofElement;
	size_t allocSize = constant ? GetCBVSize(mSize) : mSize;


	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(allocSize);
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

	mData = std::make_shared<BYTE[]>(mSize);

	if (initialData != nullptr) {
		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = initialData;
		subresourceData.RowPitch = mSize;
		subresourceData.SlicePitch = mSize;

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &barrier);

		UpdateSubresources(commandList.Get(), mBuffer.Get(), mUploadBuffer.Get(), 0, 0, 1, &subresourceData);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandList->ResourceBarrier(1, &barrier);
	}

	std::wstring name{ L"Upload Size - " + std::to_wstring(mSize) };
	std::wstring name2{ L"Default Size - " + std::to_wstring(mSize) };
	
	mUploadBuffer->SetName(name.c_str());
	mBuffer->SetName(name2.c_str());
}

DefaultBuffer::DefaultBuffer(ComPtr<ID3D12Device> device, size_t unitSize, size_t numofElement, bool constant){
	CrashExp(unitSize * numofElement > 0, "Size must be greater than 0");

	mElementSize = unitSize;
	mSize = unitSize * numofElement;
	size_t allocSize = constant ? GetCBVSize(mSize) : mSize;

	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(allocSize);

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

	mData = std::make_shared<BYTE[]>(mSize);

	std::wstring name{ L"Upload Size - " + std::to_wstring(mSize) };
	std::wstring name2{ L"Default Size - " + std::to_wstring(mSize) };

	mUploadBuffer->SetName(name.c_str());
	mBuffer->SetName(name2.c_str());
}

DefaultBuffer::~DefaultBuffer() {

}

DefaultBuffer::DefaultBuffer(const DefaultBuffer& other) {
	mBuffer = other.mBuffer;
	mUploadBuffer = other.mUploadBuffer;
	mElementSize = other.mElementSize;
	mSize = other.mSize;
	mData = other.mData;
}

DefaultBuffer& DefaultBuffer::operator=(const DefaultBuffer& other) {
	if (this != &other) {
		mBuffer = other.mBuffer;
		mUploadBuffer = other.mUploadBuffer;
		mElementSize = other.mElementSize;
		mSize = other.mSize;
		mData = other.mData;
	}
	return *this;
}

DefaultBuffer::DefaultBuffer(DefaultBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mUploadBuffer = std::move(other.mUploadBuffer);
		mElementSize = other.mElementSize;
		mSize = other.mSize;
		mData = std::move(other.mData);
	}
}

DefaultBuffer& DefaultBuffer::operator=(DefaultBuffer&& other) noexcept {
	if (this != &other) {
		mBuffer = std::move(other.mBuffer);
		mUploadBuffer = std::move(other.mUploadBuffer);
		mElementSize = other.mElementSize;
		mSize = other.mSize;
		mData = std::move(other.mData);
	}
	return *this;
}

bool DefaultBuffer::Empty() const {
	return mElementSize == 0;
}

BYTE* DefaultBuffer::Data() {
	return mData.get();
}

void DefaultBuffer::Upload(ComPtr<ID3D12GraphicsCommandList> commandList) {
	D3D12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST) };
	commandList->ResourceBarrier(1, &barrier);

	D3D12_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pData = mData.get();
	subresourceData.RowPitch = mSize;
	subresourceData.SlicePitch = mSize;

	::UpdateSubresources(commandList.Get(), mBuffer.Get(), mUploadBuffer.Get(), 0, 0, 1, &subresourceData);
	
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &barrier);
}


void DefaultBuffer::Upload(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator begin, DefaultBufferCPUIterator end, size_t dstBegin) {
	D3D12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST) };
	commandList->ResourceBarrier(1, &barrier);

	auto size = end - begin;
	auto offset = begin - CPUBegin();

	BYTE* data{ nullptr };
	CheckHR(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data)));
	std::memcpy(data, *begin, size);
	mUploadBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(mBuffer.Get(), dstBegin, mUploadBuffer.Get(), offset , size);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &barrier);
}

