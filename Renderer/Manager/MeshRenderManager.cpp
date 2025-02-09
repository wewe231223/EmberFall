#include "pch.h"
#include "MeshRenderManager.h"

MeshRenderManager::MeshRenderManager(ComPtr<ID3D12Device> device) {
	mPlaneMeshBuffer = DefaultBuffer(device, sizeof(PlainModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
}

// 복사 1 
void MeshRenderManager::AppendPlaneMeshContext(GraphicsShaderBase* shader, PlainMesh* mesh, const PlainModelContext& world) {
	mPlainMeshContexts[shader][mesh].emplace_back(world);
}

// 복사 2 
void MeshRenderManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
	DefaultBufferCPUIterator it{ mPlaneMeshBuffer.CPUBegin() };

	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(PlainModelContext));
		}
	}

	DefaultBufferGPUIterator gpuIt{ mPlaneMeshBuffer.GPUBegin() };
	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		shader->SetShader(commandList);
		for (auto& [mesh, worlds] : meshContexts) {
			
			mesh->Bind(commandList);

			commandList->SetGraphicsRootConstantBufferView(1, *gpuIt);

			++gpuIt;
		}
	}


}

void MeshRenderManager::Reset(){
	mPlainMeshContexts.clear(); 
}
