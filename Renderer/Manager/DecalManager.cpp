#include "pch.h"
#include "DecalManager.h"
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

void DecalManager::Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	mDecalInfoBuffer = DefaultBuffer(device, sizeof(DecalInfo), MAX_DECAL_COUNT);
}

void DecalManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mDecalInfoBuffer.Upload(commandList); 
	mDecalShader->SetGPassShader(commandList);

	// bind


	commandList->DrawInstanced(1, mDecalCount, 0, 0);
}

void DecalManager::AppendDecalInfo(const std::vector<DecalInfo>& decalInfos) {
	std::memcpy(*mDecalInfoBuffer.CPUBegin(), decalInfos.data(),
		decalInfos.size() < MAX_DECAL_COUNT ? decalInfos.size() * sizeof(DecalInfo) : MAX_DECAL_COUNT * sizeof(DecalInfo));
}
