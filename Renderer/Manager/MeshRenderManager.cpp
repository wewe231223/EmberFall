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

	auto& [shader, meshContexts] = *mBonedMeshContexts.begin();
	shader->SetShader(commandList);

	//auto& [shader, meshContexts] = *mPlainMeshContexts.begin();
	//shader->SetShader(commandList);
}

// 복사 2 
void MeshRenderManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList) {
	 //MeshRenderManager::RenderPlainMesh(commandList);
	MeshRenderManager::RenderBonedMesh(commandList);
}

void MeshRenderManager::Reset(){
	mBoneCounter = 0;
	mBoneTransforms.clear();
	mBonedMeshContexts.clear();
	mPlainMeshContexts.clear(); 
}

void MeshRenderManager::RenderPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList) {
	DefaultBufferGPUIterator gpuIt{ mPlainMeshBuffer.GPUBegin() };

	auto& [shader, meshContexts] = *mPlainMeshContexts.begin();

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

	for (auto& [shader, meshContexts] : mPlainMeshContexts | std::views::drop(1)) {
		shader->SetShader(commandList);
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

void MeshRenderManager::RenderBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList) {

	auto& [shader, meshContexts] = *mBonedMeshContexts.begin();
	shader->SetShader(commandList);

	DefaultBufferGPUIterator boneIt{ mAnimationBuffer.GPUBegin() };
	// Animation Buffer Set
	commandList->SetGraphicsRootShaderResourceView(4, *boneIt);


	DefaultBufferGPUIterator gpuIt{ mBonedMeshBuffer.GPUBegin() };

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

	for (auto& [shader, meshContexts] : mBonedMeshContexts | std::views::drop(1)) {
		shader->SetShader(commandList);
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
