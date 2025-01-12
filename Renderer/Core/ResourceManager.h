#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ResourceManager.h
// 2025.01.12 김승범   - 리소스를 관리하는 ResourceManager 클래스를 정의함.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../Utility/DirectXInclude.h"

class ResourceManager {
public:
	ResourceManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	~ResourceManager();

	ResourceManager(const ResourceManager& other) = delete;
	ResourceManager& operator=(const ResourceManager& other) = delete;

	ResourceManager(ResourceManager&& other) noexcept;
	ResourceManager& operator=(ResourceManager&& other) noexcept;
public:
private:
	ComPtr<ID3D12DescriptorHeap> mTextureHeap{ nullptr };
};