#include "pch.h"
#include "MeshRenderManager.h"
#include <ranges>

MeshRenderManager::MeshRenderManager(ComPtr<ID3D12Device> device) {
	mPlainMeshBuffer = DefaultBuffer(device, sizeof(PlainModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
}

// 복사 1 
void MeshRenderManager::AppendPlaneMeshContext(GraphicsShaderBase* shader, PlainMesh* mesh, const PlainModelContext& world) {
	mPlainMeshContexts[shader][mesh].emplace_back(world);
}

void MeshRenderManager::PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList) {
	DefaultBufferCPUIterator it{ mPlainMeshBuffer.CPUBegin() };

	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(PlainModelContext));
			it += worlds.size();
		}
	}

	mPlainMeshBuffer.Upload(commandList);

	auto& [shader, meshContexts] = *mPlainMeshContexts.begin();
	shader->SetShader(commandList);
}

// 복사 2 
void MeshRenderManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
	DefaultBufferGPUIterator gpuIt{ mPlainMeshBuffer.GPUBegin() };

	auto& [shader, meshContexts] = *mPlainMeshContexts.begin();

	for (auto& [mesh, worlds] : meshContexts) {
		mesh->Bind(commandList);

		commandList->SetGraphicsRootShaderResourceView(1, *gpuIt);

		if (mesh->GetIndexed()) {
			commandList->DrawIndexedInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0, 0);
		}
		else {
			commandList->DrawInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0);
		}

		gpuIt += worlds.size();
	}

	for (auto& [shader, meshContexts] : mPlainMeshContexts | std::views::drop(1) ) {
		shader->SetShader(commandList);
		for (auto& [mesh, worlds] : meshContexts) {
			
			mesh->Bind(commandList);

			commandList->SetGraphicsRootShaderResourceView(1, *gpuIt);

			if (mesh->GetIndexed()) {
				commandList->DrawIndexedInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0, 0);
			}
			else {
				commandList->DrawInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0);
			}

			gpuIt += worlds.size();
		}
	}


}

void MeshRenderManager::Reset(){
	mPlainMeshContexts.clear(); 
}
