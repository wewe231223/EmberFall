#pragma once 
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Core/Shader.h"
#include "../Utility/Defines.h"

class DecalManager {
	static constexpr size_t MAX_DECAL_COUNT = 1000; 
public:
	DecalManager() = default;
	~DecalManager() = default;

	DecalManager(const DecalManager&) = default;
	DecalManager& operator=(const DecalManager&) = default;

	DecalManager(DecalManager&&) = default;
	DecalManager& operator=(DecalManager&&) = default;

public:
	void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	void Render(ComPtr<ID3D12GraphicsCommandList> commandList);

	void AppendDecalInfo(const std::vector<DecalInfo>& decalInfos);
	
private:
	std::shared_ptr<GraphicsShaderBase> mDecalShader{};

	DefaultBuffer mDecalInfoBuffer{}; 
	UINT mDecalCount{};
};