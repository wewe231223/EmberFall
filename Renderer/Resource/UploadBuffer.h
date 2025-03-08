#pragma once 
#include "../Utility/DirectXInclude.h"

class UploadBufferCPUIterator {
public:
	using value_type = BYTE*;
	using difference_type = std::ptrdiff_t;
public:
	UploadBufferCPUIterator() = default;
	explicit UploadBufferCPUIterator(BYTE* ptr, size_t increasement) : mPtr(ptr), mIncreasement(increasement) {};
public:
	UploadBufferCPUIterator& operator++() {
		mPtr += mIncreasement;
		return *this;
	}

	UploadBufferCPUIterator operator++(int) {
		UploadBufferCPUIterator temp = *this;
		mPtr += mIncreasement;
		return temp;
	}

	UploadBufferCPUIterator& operator--() {
		mPtr -= mIncreasement;
		return *this;
	}

	UploadBufferCPUIterator operator--(int) {
		UploadBufferCPUIterator temp = *this;
		mPtr -= mIncreasement;
		return temp;
	}

	UploadBufferCPUIterator& operator+=(difference_type n) {
		mPtr += n * mIncreasement;
		return *this;
	}

	UploadBufferCPUIterator operator+(difference_type n) const {
		return UploadBufferCPUIterator(mPtr + n * mIncreasement, mIncreasement);
	}

	UploadBufferCPUIterator& operator-=(difference_type n) {
		mPtr -= n * mIncreasement;
		return *this;
	}

	UploadBufferCPUIterator operator-(difference_type n) const {
		return UploadBufferCPUIterator(mPtr - n * mIncreasement, mIncreasement);
	}

	difference_type operator-(const UploadBufferCPUIterator& other) const {
		return mPtr - other.mPtr;
	}

	value_type operator*() {
		return mPtr;
	}
private:
	BYTE* mPtr{ nullptr };
	size_t mIncreasement{ 0 };
};

class UploadBufferGPUIterator {
public:
	using value_type = D3D12_GPU_VIRTUAL_ADDRESS;
	using difference_type = std::ptrdiff_t;
public:
	UploadBufferGPUIterator() = default;
	explicit UploadBufferGPUIterator(D3D12_GPU_VIRTUAL_ADDRESS ptr, size_t increasement) : mPtr(ptr), mIncreasement(increasement) {};
public:
	UploadBufferGPUIterator& operator++() {
		mPtr += mIncreasement;
		return *this;
	}

	UploadBufferGPUIterator operator++(int) {
		UploadBufferGPUIterator temp = *this;
		mPtr += mIncreasement;
		return temp;
	}

	UploadBufferGPUIterator& operator--() {
		mPtr -= mIncreasement;
		return *this;
	}

	UploadBufferGPUIterator operator--(int) {
		UploadBufferGPUIterator temp = *this;
		mPtr -= mIncreasement;
		return temp;
	}

	UploadBufferGPUIterator& operator+=(difference_type n) {
		mPtr += n * mIncreasement;
		return *this;
	}

	UploadBufferGPUIterator operator+(difference_type n) const {
		return UploadBufferGPUIterator(mPtr + n * mIncreasement, mIncreasement);
	}

	UploadBufferGPUIterator& operator-=(difference_type n) {
		mPtr -= n * mIncreasement;
		return *this;
	}

	UploadBufferGPUIterator operator-(difference_type n) const {
		return UploadBufferGPUIterator(mPtr - n * mIncreasement, mIncreasement);
	}

	difference_type operator-(const UploadBufferGPUIterator& other) const {
		return mPtr - other.mPtr;
	}

	value_type operator*() {
		return mPtr;
	}
private:
	D3D12_GPU_VIRTUAL_ADDRESS mPtr{ 0 };
	size_t mIncreasement{ 0 };
};

class UploadBuffer {
public:
	UploadBuffer() = default;
	UploadBuffer(ComPtr<ID3D12Device> device, size_t unitSize, size_t numofElement, void* initialData = nullptr);

	~UploadBuffer() = default;

	UploadBuffer(const UploadBuffer& other);
	UploadBuffer& operator=(const UploadBuffer& other);

	UploadBuffer(UploadBuffer&& other) noexcept;
	UploadBuffer& operator=(UploadBuffer&& other) noexcept;
public:
	BYTE* Data();

	UploadBufferCPUIterator CPUBegin() { return UploadBufferCPUIterator(mData, mElementSize); }
	UploadBufferCPUIterator CPUEnd() { return UploadBufferCPUIterator(mData + mSize, mElementSize); }

	UploadBufferGPUIterator GPUBegin() { return UploadBufferGPUIterator(mBuffer->GetGPUVirtualAddress(), mElementSize); }
	UploadBufferGPUIterator GPUEnd() { return UploadBufferGPUIterator(mBuffer->GetGPUVirtualAddress() + mSize, mElementSize); }
private:
	ComPtr<ID3D12Resource> mBuffer{ nullptr };

	BYTE* mData{ nullptr };

	size_t mSize{ 0 };
	size_t mElementSize{ 0 };
};