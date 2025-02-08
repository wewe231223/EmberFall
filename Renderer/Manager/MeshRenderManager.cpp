#include "pch.h"
#include "MeshRenderManager.h"

MeshRenderManager::MeshRenderManager(ComPtr<ID3D12Device> device) {
	mPlaneMeshBuffer = DefaultBuffer(device, MAX_INSTANCE_COUNT<size_t> * sizeof(SimpleMath::Matrix));
}

// 복사 1 
void MeshRenderManager::AppendPlaneMeshContext(GraphicsShaderBase* shader, PlainMesh* mesh, const SimpleMath::Matrix& world) {
	mPlainMeshContexts[shader][mesh].emplace_back(world);
}

// 복사 2 
void MeshRenderManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mPlaneMeshBuffer.ResetAccumulation();

	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		shader->SetShader(commandList);

		for (auto& [mesh, worlds] : meshContexts) {
			mesh->Bind(commandList);
			
			// copy 
			mPlaneMeshBuffer.AccumulateData(commandList, worlds.data(), worlds.size() * sizeof(SimpleMath::Matrix));


			// set 

			// render
			if (mesh->GetIndexed()) {
				commandList->DrawIndexedInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0, 0);
			}
			else {
				commandList->DrawInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0);
			}

		}
	}
}

void MeshRenderManager::Reset(){
	mPlainMeshContexts.clear(); 
}
