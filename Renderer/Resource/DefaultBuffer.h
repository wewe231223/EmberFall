#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DefaultBuffer.h
// 2025.01.12 김승범   - Default 버퍼를 생성하는 DefaultBuffer 클래스를 정의함.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Utility/DirectXInclude.h"
#include "../Utility/Crash.h"
#include "../Utility/Exceptions.h"

template<typename T> 
class DefaultBuffer {
public:
	DefaultBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, size_t count, T* initialData = nullptr) {
		CrashExp(count > 0, "Count must be greater than 0");

		mCount = count;

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) * count);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&mBuffer)
		));

		heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		ThrowIfFailed(device->CreateCommittedResource(
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
			subresourceData.RowPitch = sizeof(T) * mCount;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			commandList->ResourceBarrier(1, &barrier);

			UpdateSubresources(commandList.Get(), mBuffer.Get(), mUploadBuffer.Get(), 0, 0, 1, &subresourceData);
			
			barrier = CD3DX12_RESOURCE_BARRIER::Transition(mBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
			commandList->ResourceBarrier(1, &barrier);
		}
	}

	~DefaultBuffer() {
		if (mBuffer != nullptr) {
			mBuffer->Unmap(0, nullptr);
		}
	}

	DefaultBuffer(const DefaultBuffer& other) = delete;
	DefaultBuffer& operator=(const DefaultBuffer& other) = delete;

	DefaultBuffer(DefaultBuffer&& other) noexcept {
		if (this != &other) {
			mBuffer = std::move(other.mBuffer);
			mUploadBuffer = std::move(other.mUploadBuffer);
			mCount = other.mCount;
		}
	}

	DefaultBuffer& operator=(DefaultBuffer&& other) noexcept {
		if (this != &other) {
			mBuffer = std::move(other.mBuffer);
			mUploadBuffer = std::move(other.mUploadBuffer);
			mCount = other.mCount;
		}
		return *this;
	}

public:
private:
	ComPtr<ID3D12Resource> mBuffer{ nullptr };
	ComPtr<ID3D12Resource> mUploadBuffer{ nullptr };
	
	T* mBufferPtr{ nullptr };

	size_t mCount{ 0 };
};
