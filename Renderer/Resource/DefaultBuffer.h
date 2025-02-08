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

class DefaultBuffer {
public:
	DefaultBuffer() = default;
	DefaultBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, size_t size, void* initialData = nullptr);
	DefaultBuffer(ComPtr<ID3D12Device> device, size_t size);
	~DefaultBuffer();

	DefaultBuffer(const DefaultBuffer& other) = delete;
	DefaultBuffer& operator=(const DefaultBuffer& other) = delete;

	DefaultBuffer(DefaultBuffer&& other) noexcept; 
	DefaultBuffer& operator=(DefaultBuffer&& other) noexcept;

public:
	bool Empty() const; 
	void Copy(ComPtr<ID3D12GraphicsCommandList> commandList, void* data, std::ptrdiff_t begin = 0, std::ptrdiff_t end = 0);
	void AccumulateData(ComPtr<ID3D12GraphicsCommandList> commandList, void* data, size_t size);
	void ResetAccumulation();
private:
	ComPtr<ID3D12Resource> mBuffer{ nullptr };
	ComPtr<ID3D12Resource> mUploadBuffer{ nullptr };

	BYTE* mBufferPtr{ nullptr };
	BYTE* mAccumulationPtr{ nullptr };

	size_t mSize{ 0 };
};
