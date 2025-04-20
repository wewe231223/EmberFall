#include "pch.h"
#include "MeshRenderManager.h"
#include "../Utility/Defines.h"
#include <ranges>

MeshRenderManager::MeshRenderManager(ComPtr<ID3D12Device> device) {
	mPlainMeshBuffer = DefaultBuffer(device, sizeof(ModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mBonedMeshBuffer = DefaultBuffer(device, sizeof(AnimationModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mAnimationBuffer = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);

	mShadowPlainMeshBuffer = DefaultBuffer(device, sizeof(ModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mShadowBonedMeshBuffer = DefaultBuffer(device, sizeof(AnimationModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mShadowAnimationBuffer = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);

	mSkeletonBoundingboxRenderShader = std::make_unique<SkeletonBBShader>();
	mSkeletonBoundingboxRenderShader->CreateShader(device);

	mStandardBoundingBoxRenderShader = std::make_unique<StandardBBShader>();
	mStandardBoundingBoxRenderShader->CreateShader(device);
}


void MeshRenderManager::AppendPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, UINT reservedSlot) {
	if ((0 != (~reservedSlot)) and mReservedSlotCounter <= MeshRenderManager::RESERVED_CONTEXT_SLOT) {
		mPlainMeshReserved[shader][mesh].emplace_back(world);
		mReservedSlotCounter++;
	}
	else {
		mPlainMeshContexts[shader][mesh].emplace_back(world);
	}
}

void MeshRenderManager::AppendBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, BoneTransformBuffer& boneTransforms) {

	AnimationModelContext context{ world.world, world.BBCenter, world.BBextents, world.material, mBoneCounter };	
	mBonedMeshContexts[shader][mesh].emplace_back(context);
	mBoneCounter += boneTransforms.boneCount; 

	mBoneTransforms.insert(mBoneTransforms.end(), std::make_move_iterator(boneTransforms.boneTransforms.begin()), std::make_move_iterator(boneTransforms.boneTransforms.begin() + boneTransforms.boneCount ));
}

void MeshRenderManager::AppendShadowPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, UINT index) {
	
	mShadowPlainMeshContexts[index][shader][mesh].emplace_back(world);
	if (index == 0) {
		mShadowMeshCounter[index + 1] += 1;
		
	}
	
	
}

void MeshRenderManager::AppendShadowBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, BoneTransformBuffer& boneTransforms) {
	AnimationModelContext context{ world.world, world.BBCenter, world.BBextents, world.material, mShadowBoneCounter };
	mShadowBonedMeshContexts[shader][mesh].emplace_back(context);
	mShadowBoneCounter += boneTransforms.boneCount;

	mShadowBoneTransforms.insert(mShadowBoneTransforms.end(), std::make_move_iterator(boneTransforms.boneTransforms.begin()), std::make_move_iterator(boneTransforms.boneTransforms.begin() + boneTransforms.boneCount));

}

void MeshRenderManager::PrepareRender(ComPtr<ID3D12GraphicsCommandList> commandList) {
	DefaultBufferCPUIterator it{ mPlainMeshBuffer.CPUBegin() };

	for (auto& [shader, meshContexts] : mPlainMeshReserved) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(ModelContext));
			it += worlds.size();
		}
	}

	it = mPlainMeshBuffer.CPUBegin() + static_cast<std::ptrdiff_t>(MeshRenderManager::RESERVED_CONTEXT_SLOT);

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

	it = mShadowPlainMeshBuffer.CPUBegin();
	for (auto& shadowPlainMeshContext : mShadowPlainMeshContexts) {
		for (auto& [shader, meshContexts] : shadowPlainMeshContext) {
			for (auto& [mesh, worlds] : meshContexts) {
				std::memcpy(*it, worlds.data(), worlds.size() * sizeof(ModelContext));
				it += worlds.size();
			}
		}
	}
	mShadowPlainMeshBuffer.Upload(commandList, mShadowPlainMeshBuffer.CPUBegin(), it);


	it = mShadowBonedMeshBuffer.CPUBegin();

	for (auto& [shader, meshContexts] : mShadowBonedMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(AnimationModelContext));
			it += worlds.size();
		}
	}

	mShadowBonedMeshBuffer.Upload(commandList, mShadowBonedMeshBuffer.CPUBegin(), it);

	it = mAnimationBuffer.CPUBegin();
	std::memcpy(*it, mBoneTransforms.data(), mBoneTransforms.size() * sizeof(SimpleMath::Matrix));
	it += mBoneTransforms.size();
	mAnimationBuffer.Upload(commandList, mAnimationBuffer.CPUBegin(), it);

	it = mShadowAnimationBuffer.CPUBegin();
	std::memcpy(*it, mShadowBoneTransforms.data(), mShadowBoneTransforms.size() * sizeof(SimpleMath::Matrix));
	it += mShadowBoneTransforms.size();
	mShadowAnimationBuffer.Upload(commandList, mShadowAnimationBuffer.CPUBegin(), it);
}

void MeshRenderManager::RenderShadowPass(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex,D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	MeshRenderManager::RenderShadowPassPlainMesh(index, commandList, tex, mat, camera);
	MeshRenderManager::RenderShadowPassBonedMesh(commandList, mat, camera);
}

// 복사 2 
void MeshRenderManager::RenderGPass(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	MeshRenderManager::RenderGPassPlainMesh(commandList, tex, mat, camera);
	MeshRenderManager::RenderGPassBonedMesh(commandList, tex, mat, camera);
}

void MeshRenderManager::Reset(){
	mBoneCounter = 0;
	mShadowBoneCounter = 0;
	mReservedSlotCounter = 0;
	mShadowMeshCounter.fill(0);
	mBoneTransforms.clear();
	mShadowBoneTransforms.clear();
	mBonedMeshContexts.clear();
	mPlainMeshReserved.clear();
	mPlainMeshContexts.clear(); 

	mShadowBonedMeshContexts.clear();
	for (auto& shadowPlainMeshContext : mShadowPlainMeshContexts) {
		shadowPlainMeshContext.clear();
	}
	
}

void MeshRenderManager::RenderShadowPassPlainMesh(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	auto gpuIt = mShadowPlainMeshBuffer.GPUBegin() + static_cast<std::ptrdiff_t>(mShadowMeshCounter[index]);

	for (auto& [shader, meshContexts] : mShadowPlainMeshContexts[index]) {
		shader->SetShadowPassShader(commandList);
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

void MeshRenderManager::RenderShadowPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	DefaultBufferGPUIterator boneIt{ mAnimationBuffer.GPUBegin() };
	DefaultBufferGPUIterator gpuIt{ mBonedMeshBuffer.GPUBegin() };

	for (auto& [shader, meshContexts] : mBonedMeshContexts) {

		shader->SetShadowPassShader(commandList);

		commandList->SetGraphicsRootConstantBufferView(0, camera);
		commandList->SetGraphicsRootShaderResourceView(2, mat);
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

void MeshRenderManager::RenderGPassPlainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	DefaultBufferGPUIterator gpuIt{ mPlainMeshBuffer.GPUBegin() };

	for (auto& [shader, meshContexts] : mPlainMeshReserved) {
		shader->SetGPassShader(commandList);
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

	gpuIt = mPlainMeshBuffer.GPUBegin() + static_cast<std::ptrdiff_t>(MeshRenderManager::RESERVED_CONTEXT_SLOT);

	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		shader->SetGPassShader(commandList);
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


#ifdef RENDER_BB

	gpuIt = mPlainMeshBuffer.GPUBegin() + static_cast<std::ptrdiff_t>(MeshRenderManager::RESERVED_CONTEXT_SLOT);


	mStandardBoundingBoxRenderShader->SetGPassShader(commandList);

	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	commandList->SetGraphicsRootConstantBufferView(0, camera);


	for (auto& [shader, meshContexts] : mPlainMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {

			commandList->SetGraphicsRootShaderResourceView(1, *gpuIt);
			commandList->DrawInstanced(1, static_cast<UINT>(worlds.size()), 0, 0);

			gpuIt += worlds.size();
		}
	}
#endif 

}

void MeshRenderManager::RenderGPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {

	DefaultBufferGPUIterator boneIt{ mAnimationBuffer.GPUBegin() };
	DefaultBufferGPUIterator gpuIt{ mBonedMeshBuffer.GPUBegin() };

	for (auto& [shader, meshContexts] : mBonedMeshContexts) {

		shader->SetGPassShader(commandList);

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

#ifdef RENDER_BB

	gpuIt = mBonedMeshBuffer.GPUBegin(); 


	mSkeletonBoundingboxRenderShader->SetGPassShader(commandList);
	
	commandList->IASetVertexBuffers(0, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST); 
	commandList->SetGraphicsRootConstantBufferView(0, camera);


	for (auto& [shader, meshContexts] : mBonedMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {

			commandList->SetGraphicsRootShaderResourceView(1, *gpuIt);
			commandList->DrawInstanced(1, static_cast<UINT>(worlds.size()), 0, 0);

			gpuIt += worlds.size(); 
		}
	}
#endif RENDER_BB

}
