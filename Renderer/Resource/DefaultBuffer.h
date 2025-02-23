#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DefaultBuffer.h
// 2025.01.12 김승범   - Default 버퍼를 생성하는 DefaultBuffer 클래스를 정의함.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iterator>
#include "../Utility/DirectXInclude.h"
#include "../Utility/Crash.h"
#include "../Utility/Exceptions.h"

class DefaultBufferCPUIterator {
public:
	using value_type = BYTE*;
	using difference_type = std::ptrdiff_t;
public:
	DefaultBufferCPUIterator() = default;
	explicit DefaultBufferCPUIterator(BYTE* ptr, size_t increasement) : mPtr(ptr), mIncreasement(increasement) {};
public:
	DefaultBufferCPUIterator& operator++() { 
		mPtr += mIncreasement; 
		return *this; 
	}

	DefaultBufferCPUIterator operator++(int) { 
		DefaultBufferCPUIterator temp = *this; 
		mPtr += mIncreasement; 
		return temp; 
	}

	DefaultBufferCPUIterator& operator--() { 
		mPtr -= mIncreasement; 
		return *this; 
	}

	DefaultBufferCPUIterator operator--(int) { 
		DefaultBufferCPUIterator temp = *this; 
		mPtr -= mIncreasement; 
		return temp; 
	}

	DefaultBufferCPUIterator& operator+=(difference_type n) { 
		mPtr += n * mIncreasement; 
		return *this; 
	}

	DefaultBufferCPUIterator operator+(difference_type n) const { 
		return DefaultBufferCPUIterator(mPtr + n * mIncreasement, mIncreasement); 
	}

	DefaultBufferCPUIterator& operator-=(difference_type n) {
		mPtr -= n * mIncreasement; 
		return *this; 
	}

	DefaultBufferCPUIterator operator-(difference_type n) const {
		return DefaultBufferCPUIterator(mPtr - n * mIncreasement, mIncreasement); 
	}

	difference_type operator-(const DefaultBufferCPUIterator& other) const {
		return mPtr - other.mPtr; 
	}

	value_type operator*() { 
		return mPtr; 
	}

	bool operator==(const DefaultBufferCPUIterator& other) const { 
		return mPtr == other.mPtr; 
	}

	bool operator<(const DefaultBufferCPUIterator& other) const { 
		return mPtr < other.mPtr; 
	}

	bool operator<=(const DefaultBufferCPUIterator& other) const { 
		return mPtr <= other.mPtr; 
	}

private:
	BYTE* mPtr{ nullptr };
	const size_t mIncreasement;
};

class DefaultBufferGPUIterator {
public:
	using value_type = D3D12_GPU_VIRTUAL_ADDRESS;
	using difference_type = std::ptrdiff_t;
public:
	DefaultBufferGPUIterator() = default;
	explicit DefaultBufferGPUIterator(D3D12_GPU_VIRTUAL_ADDRESS ptr, size_t increasement) : mPtr(ptr), mIncreasement(increasement) {};
public:
	DefaultBufferGPUIterator& operator++() {
		mPtr += mIncreasement;
		return *this;
	}

	DefaultBufferGPUIterator operator++(int) {
		DefaultBufferGPUIterator temp = *this;
		mPtr += mIncreasement;
		return temp;
	}

	DefaultBufferGPUIterator& operator--() {
		mPtr -= mIncreasement;
		return *this;
	}

	DefaultBufferGPUIterator operator--(int) {
		DefaultBufferGPUIterator temp = *this;
		mPtr -= mIncreasement;
		return temp;
	}

	DefaultBufferGPUIterator& operator+=(difference_type n) {
		mPtr += n * mIncreasement;
		return *this;
	}

	DefaultBufferGPUIterator operator+(difference_type n) const {
		return DefaultBufferGPUIterator(mPtr + n * mIncreasement, mIncreasement);
	}

	DefaultBufferGPUIterator& operator-=(difference_type n) {
		mPtr -= n * mIncreasement;
		return *this;
	}

	DefaultBufferGPUIterator operator-(difference_type n) const {
		return DefaultBufferGPUIterator(mPtr - n * mIncreasement, mIncreasement);
	}

	difference_type operator-(const DefaultBufferGPUIterator& other) const {
		return mPtr - other.mPtr;
	}

	value_type operator*() {
		return mPtr;
	}

	bool operator==(const DefaultBufferGPUIterator& other) const {
		return mPtr == other.mPtr;
	}

	bool operator<(const DefaultBufferGPUIterator& other) const {
		return mPtr < other.mPtr;
	}

	bool operator<=(const DefaultBufferGPUIterator& other) const {
		return mPtr <= other.mPtr;
	}
private:
	D3D12_GPU_VIRTUAL_ADDRESS mPtr{ 0 };
	const size_t mIncreasement;
};


class DefaultBuffer {
public:
	DefaultBuffer() = default;
	DefaultBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, size_t unitSize, size_t numofElement, const void* initialData = nullptr, bool constant = false);
	DefaultBuffer(ComPtr<ID3D12Device> device, size_t unitSize, size_t numofElement, bool constant = false);
	~DefaultBuffer();

	DefaultBuffer(const DefaultBuffer& other);
	DefaultBuffer& operator=(const DefaultBuffer& other);

	DefaultBuffer(DefaultBuffer&& other) noexcept; 
	DefaultBuffer& operator=(DefaultBuffer&& other) noexcept;

public:
	bool Empty() const; 

	DefaultBufferCPUIterator CPUBegin() { return DefaultBufferCPUIterator(mData.get(), mElementSize); }
	DefaultBufferCPUIterator CPUEnd() { return DefaultBufferCPUIterator(mData.get() + mSize, mElementSize); }

	DefaultBufferGPUIterator GPUBegin() { return DefaultBufferGPUIterator(mBuffer->GetGPUVirtualAddress(), mElementSize); }
	DefaultBufferGPUIterator GPUEnd() { return DefaultBufferGPUIterator(mBuffer->GetGPUVirtualAddress() + mSize, mElementSize); }

	BYTE* Data(); 

	void Upload(ComPtr<ID3D12GraphicsCommandList> commandList);
	void Upload(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator begin, DefaultBufferCPUIterator end, size_t dstBegin = 0);
	
private:
	ComPtr<ID3D12Resource> mBuffer{ nullptr };
	ComPtr<ID3D12Resource> mUploadBuffer{ nullptr };

	std::shared_ptr<BYTE[]> mData{ nullptr };

	size_t mSize{ 0 };
	size_t mElementSize{ 0 };
};
