#include "pch.h"
#include "MeshRenderManager.h"
#include <ranges>

MeshRenderManager::MeshRenderManager(ComPtr<ID3D12Device> device) {
	mPlainMeshBuffer = DefaultBuffer(device, sizeof(ModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mBonedMeshBuffer = DefaultBuffer(device, sizeof(AnimationModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mAnimationBuffer = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);
}

// 복사 1 
void MeshRenderManager::AppendPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world) {
	mPlainMeshContexts[shader][mesh].emplace_back(world);
}

// 여기서 부터 시작.. 
void MeshRenderManager::AppendBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, std::vector<SimpleMath::Matrix>& boneTransform) {

	AnimationModelContext context{ world.world, world.material, mBoneCounter };	
	mBonedMeshContexts[shader][mesh].emplace_back(context);
	mBoneCounter += static_cast<UINT>(boneTransform.size());

	mBoneTransforms.insert(mBoneTransforms.end(), std::make_move_iterator(boneTransform.begin()), std::make_move_iterator(boneTransform.end()));
}

void MeshRenderManager::PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList) {
	DefaultBufferCPUIterator it{ mPlainMeshBuffer.CPUBegin() };

	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(ModelContext));
			it += worlds.size();
		}
	}

	mPlainMeshBuffer.Upload(commandList, mPlainMeshBuffer.CPUBegin(), it);

	it = mBonedMeshBuffer.CPUBegin();

	for (auto& [shader, meshContexts] : mBonedMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(AnimationModelContext));
			it += worlds.size();
		}
	}

	mBonedMeshBuffer.Upload(commandList, mBonedMeshBuffer.CPUBegin(), it);

	it = mAnimationBuffer.CPUBegin();
	std::memcpy(*it, mBoneTransforms.data(), mBoneTransforms.size() * sizeof(SimpleMath::Matrix));
	it += mBoneTransforms.size();
	mAnimationBuffer.Upload(commandList, mAnimationBuffer.CPUBegin(), it);
}

// 복사 2 
void MeshRenderManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	MeshRenderManager::RenderPlainMesh(commandList,tex,mat,camera);
	MeshRenderManager::RenderBonedMesh(commandList,tex,mat,camera);
}

void MeshRenderManager::Reset(){
	mBoneCounter = 0;
	mBoneTransforms.clear();
	mBonedMeshContexts.clear();
	mPlainMeshContexts.clear(); 
}

void MeshRenderManager::RenderPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	DefaultBufferGPUIterator gpuIt{ mPlainMeshBuffer.GPUBegin() };

	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		shader->SetShader(commandList);
		commandList->SetGraphicsRootConstantBufferView(0, camera); 
		commandList->SetGraphicsRootShaderResourceView(2, mat);
		commandList->SetGraphicsRootDescriptorTable(3, tex);

		for (auto& [mesh, worlds] : meshContexts) {

			mesh->Bind(commandList, shader->GetAttribute());

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

void MeshRenderManager::RenderBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {

	DefaultBufferGPUIterator boneIt{ mAnimationBuffer.GPUBegin() };
	DefaultBufferGPUIterator gpuIt{ mBonedMeshBuffer.GPUBegin() };

	for (auto& [shader, meshContexts] : mBonedMeshContexts) {

		shader->SetShader(commandList);

		commandList->SetGraphicsRootConstantBufferView(0, camera);
		commandList->SetGraphicsRootShaderResourceView(2, mat);
		commandList->SetGraphicsRootDescriptorTable(3, tex);
		commandList->SetGraphicsRootShaderResourceView(4, *boneIt);

		for (auto& [mesh, worlds] : meshContexts) {
			mesh->Bind(commandList, shader->GetAttribute());

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
