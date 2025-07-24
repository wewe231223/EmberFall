#include "pch.h"
#include "MeshRenderManager.h"
#include "../Utility/Defines.h"
#include <ranges>

MeshRenderManager::MeshRenderManager(ComPtr<ID3D12Device> device) {


	mPlainMeshBuffer = DefaultBuffer(device, sizeof(ModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mBonedMeshBuffer = DefaultBuffer(device, sizeof(AnimationModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mAnimationBuffer = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);
	mTerrainMeshBuffer = DefaultBuffer(device, sizeof(TerrainSegmentContext), MeshRenderManager::MAX_TERRAIN_SEGMENT_COUNT<size_t>);
	mPrevAnimationBuffer = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);

	mShadowPlainMeshBuffer = DefaultBuffer(device, sizeof(ModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mShadowBonedMeshBuffer[0] = DefaultBuffer(device, sizeof(AnimationModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mShadowBonedMeshBuffer[1] = DefaultBuffer(device, sizeof(AnimationModelContext), MeshRenderManager::MAX_INSTANCE_COUNT<size_t>);
	mShadowAnimationBuffer[0] = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);
	mShadowAnimationBuffer[1] = DefaultBuffer(device, sizeof(SimpleMath::Matrix), MeshRenderManager::MAX_BONE_COUNT<size_t>);
	mShadowTerrainMeshBuffer = DefaultBuffer(device, sizeof(TerrainSegmentContext), MeshRenderManager::MAX_TERRAIN_SEGMENT_COUNT<size_t>);

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

void MeshRenderManager::AppendBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, BoneTransformBuffer& boneTransforms, float dissolveOffset) {

	AnimationModelContext context{ world.prevWorld, world.world, world.BBCenter, world.BBextents, world.material, dissolveOffset, mBoneCounter };
	mBonedMeshContexts[shader][mesh].emplace_back(context);
	mBoneCounter += boneTransforms.boneCount;

	mPrevBoneTransforms.insert(mPrevBoneTransforms.end(), std::make_move_iterator(boneTransforms.prevBoneTransforms.begin()), std::make_move_iterator(boneTransforms.prevBoneTransforms.begin() + boneTransforms.boneCount));
	mBoneTransforms.insert(mBoneTransforms.end(), std::make_move_iterator(boneTransforms.boneTransforms.begin()), std::make_move_iterator(boneTransforms.boneTransforms.begin() + boneTransforms.boneCount));

	
}

void MeshRenderManager::AppendShadowPlaneMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, UINT index) {
	mShadowPlainMeshContexts[index][shader][mesh].emplace_back(world);
	if (index == 0) {
		mShadowMeshCounter[index + 1] += 1;
	}
}

void MeshRenderManager::AppendShadowBonedMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const ModelContext& world, BoneTransformBuffer& boneTransforms, UINT index) {
	AnimationModelContext context{ world.prevWorld, world.world, world.BBCenter, world.BBextents, world.material, 0, mShadowBoneCounter[index]};
	mShadowBonedMeshContexts[index][shader][mesh].emplace_back(context);
	mShadowBoneCounter[index] += boneTransforms.boneCount;
	
	mShadowBoneTransforms[index].insert(mShadowBoneTransforms[index].end(), std::make_move_iterator(boneTransforms.boneTransforms.begin()), std::make_move_iterator(boneTransforms.boneTransforms.begin() + boneTransforms.boneCount));

}

void MeshRenderManager::RegisterTerrainCPPointBuffer(DefaultBufferGPUIterator terrainCPPointBuffer) {
	mTerrainCPPointBuffer = terrainCPPointBuffer;
}

void MeshRenderManager::AppendTerrainMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const TerrainSegmentContext& world) {
	mTerrainMeshContexts[shader][mesh].emplace_back(world);
}

void MeshRenderManager::AppendShadowTerrainMeshContext(GraphicsShaderBase* shader, Mesh* mesh, const TerrainSegmentContext& world, UINT index) {
	mShadowTerrainMeshContexts[index][shader][mesh].emplace_back(world);
	if (index == 0) {
		mShadowTerrainMeshCounter[index + 1] += 1;
	}
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

	it = mTerrainMeshBuffer.CPUBegin();

	for (auto& [shader, meshContexts] : mTerrainMeshContexts) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(ModelContext));
			it += worlds.size();
		}
	}

	mTerrainMeshBuffer.Upload(commandList, mTerrainMeshBuffer.CPUBegin(), it);

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


	it = mShadowTerrainMeshBuffer.CPUBegin();

	for (auto& shadowTerrainMeshContext : mShadowTerrainMeshContexts) {
		for (auto& [shader, meshContexts] : shadowTerrainMeshContext) {
			for (auto& [mesh, worlds] : meshContexts) {
				std::memcpy(*it, worlds.data(), worlds.size() * sizeof(ModelContext));
				it += worlds.size();
			}
		}
	}

	mShadowTerrainMeshBuffer.Upload(commandList, mShadowTerrainMeshBuffer.CPUBegin(), it);


	it = mShadowBonedMeshBuffer[0].CPUBegin();
	for (auto& [shader, meshContexts] : mShadowBonedMeshContexts[0]) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(AnimationModelContext));
			it += worlds.size();
		}
	}
	mShadowBonedMeshBuffer[0].Upload(commandList, mShadowBonedMeshBuffer[0].CPUBegin(), it);


	it = mShadowBonedMeshBuffer[1].CPUBegin();
	for (auto& [shader, meshContexts] : mShadowBonedMeshContexts[1]) {
		for (auto& [mesh, worlds] : meshContexts) {
			std::memcpy(*it, worlds.data(), worlds.size() * sizeof(AnimationModelContext));
			it += worlds.size();
		}
	}
	mShadowBonedMeshBuffer[1].Upload(commandList, mShadowBonedMeshBuffer[1].CPUBegin(), it);


	it = mAnimationBuffer.CPUBegin();
	std::memcpy(*it, mBoneTransforms.data(), mBoneTransforms.size() * sizeof(SimpleMath::Matrix));
	it += mBoneTransforms.size();
	mAnimationBuffer.Upload(commandList, mAnimationBuffer.CPUBegin(), it);
	
	it = mPrevAnimationBuffer.CPUBegin();
	std::memcpy(*it, mPrevBoneTransforms.data(), mPrevBoneTransforms.size() * sizeof(SimpleMath::Matrix));
	it += mPrevBoneTransforms.size();
	mPrevAnimationBuffer.Upload(commandList, mPrevAnimationBuffer.CPUBegin(), it);

	it = mShadowAnimationBuffer[0].CPUBegin();

	std::memcpy(*it, mShadowBoneTransforms[0].data(), mShadowBoneTransforms[0].size() * sizeof(SimpleMath::Matrix));
	it += mShadowBoneTransforms[0].size();

	mShadowAnimationBuffer[0].Upload(commandList, mShadowAnimationBuffer[0].CPUBegin(), it);

	it = mShadowAnimationBuffer[1].CPUBegin();

	std::memcpy(*it, mShadowBoneTransforms[1].data(), mShadowBoneTransforms[1].size() * sizeof(SimpleMath::Matrix));
	it += mShadowBoneTransforms[1].size();

	mShadowAnimationBuffer[1].Upload(commandList, mShadowAnimationBuffer[1].CPUBegin(), it);
}

void MeshRenderManager::RenderShadowPass(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex,D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	MeshRenderManager::RenderShadowPassTerrainMesh(index, commandList, tex, mat, camera);
	MeshRenderManager::RenderShadowPassPlainMesh(index, commandList, tex, mat, camera);
	MeshRenderManager::RenderShadowPassBonedMesh(index, commandList, mat, camera);
}

// 복사 2 
void MeshRenderManager::RenderGPass(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera, bool renderBB) {
	mRenderBB = renderBB; 

	MeshRenderManager::RenderGPassPlainMesh(commandList, tex, mat, camera);
	MeshRenderManager::RenderGPassTerrainMesh(commandList, tex, mat, camera);
	MeshRenderManager::RenderGPassBonedMesh(commandList, tex, mat, camera);
}

void MeshRenderManager::Reset(){
	mBoneCounter = 0;
	for (UINT& shadowBoneCounter : mShadowBoneCounter) {
		shadowBoneCounter = 0;
	};

	mReservedSlotCounter = 0;
	mShadowMeshCounter.fill(0);
	mBoneTransforms.clear();
	mPrevBoneTransforms.clear();
	for (auto& shadowBoneTransform : mShadowBoneTransforms) {
		shadowBoneTransform.clear();
	}
	mBonedMeshContexts.clear();
	mPlainMeshReserved.clear();
	mPlainMeshContexts.clear(); 
	
	mTerrainMeshContexts.clear();
	mShadowTerrainMeshCounter.fill(0);

	for (auto& shadowTerrainMeshContext : mShadowTerrainMeshContexts) {
		shadowTerrainMeshContext.clear();
	}

	for (auto& shadowBonedMeshContext : mShadowBonedMeshContexts) {
		shadowBonedMeshContext.clear();
	}


	for (auto& shadowPlainMeshContext : mShadowPlainMeshContexts) {
		shadowPlainMeshContext.clear();
	}
}

void MeshRenderManager::RenderShadowPassTerrainMesh(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	auto gpuIt = mShadowTerrainMeshBuffer.GPUBegin() + static_cast<std::ptrdiff_t>(mShadowTerrainMeshCounter[index]);

	for (auto& [shader, meshContexts] : mShadowTerrainMeshContexts[index]) {
		shader->SetShadowPassShader(commandList);
		commandList->SetGraphicsRootConstantBufferView(0, camera);
		commandList->SetGraphicsRootShaderResourceView(1, *mTerrainCPPointBuffer);
		commandList->SetGraphicsRootShaderResourceView(3, mat);
		commandList->SetGraphicsRootDescriptorTable(4, tex);

		for (auto& [mesh, worlds] : meshContexts) {

			mesh->Bind(commandList, shader->GetAttribute());

			commandList->SetGraphicsRootShaderResourceView(2, *gpuIt);

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

void MeshRenderManager::RenderShadowPassBonedMesh(UINT index, ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	DefaultBufferGPUIterator boneIt{ mShadowAnimationBuffer[index].GPUBegin()};
	DefaultBufferGPUIterator gpuIt{ mShadowBonedMeshBuffer[index].GPUBegin()};

	for (auto& [shader, meshContexts] : mShadowBonedMeshContexts[index]) {

		shader->SetShadowPassShader(commandList);

		commandList->SetGraphicsRootConstantBufferView(0, camera);
		commandList->SetGraphicsRootShaderResourceView(2, mat);
		commandList->SetGraphicsRootShaderResourceView(4, *boneIt);
		commandList->SetGraphicsRootShaderResourceView(5, *boneIt);

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

void MeshRenderManager::RenderGPassTerrainMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {
	DefaultBufferGPUIterator gpuIt{ mTerrainMeshBuffer.GPUBegin() };

	for (auto& [shader, meshContexts] : mTerrainMeshContexts) {
		shader->SetGPassShader(commandList);

		commandList->SetGraphicsRootConstantBufferView(0, camera);
		commandList->SetGraphicsRootShaderResourceView(1, *mTerrainCPPointBuffer);
		commandList->SetGraphicsRootShaderResourceView(3, mat);
		commandList->SetGraphicsRootDescriptorTable(4, tex);


		for (auto& [mesh, worlds] : meshContexts) {

			mesh->Bind(commandList, shader->GetAttribute());

			commandList->SetGraphicsRootShaderResourceView(2, *gpuIt);

			if (mesh->GetIndexed()) {
				commandList->DrawIndexedInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0, 0);
			}
			else {
				commandList->DrawInstanced(mesh->GetUnitCount(), static_cast<UINT>(worlds.size()), 0, 0);
			}

			gpuIt += worlds.size();
		}
	}


	if (mRenderBB) {
		gpuIt = mTerrainMeshBuffer.GPUBegin();


		mStandardBoundingBoxRenderShader->SetGPassShader(commandList);

		commandList->IASetVertexBuffers(0, 0, nullptr);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->SetGraphicsRootConstantBufferView(0, camera);


		for (auto& [shader, meshContexts] : mTerrainMeshContexts) {
			for (auto& [mesh, worlds] : meshContexts) {

				commandList->SetGraphicsRootShaderResourceView(1, *gpuIt);
				commandList->DrawInstanced(1, static_cast<UINT>(worlds.size()), 0, 0);

				gpuIt += worlds.size();
			}
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

	if(mRenderBB){
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
	}

}

void MeshRenderManager::RenderGPassBonedMesh(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS mat, D3D12_GPU_VIRTUAL_ADDRESS camera) {

	DefaultBufferGPUIterator prevBoneIt{ mPrevAnimationBuffer.GPUBegin() };
	DefaultBufferGPUIterator boneIt{ mAnimationBuffer.GPUBegin() };
	DefaultBufferGPUIterator gpuIt{ mBonedMeshBuffer.GPUBegin() };


	for (auto& [shader, meshContexts] : mBonedMeshContexts) {

		shader->SetGPassShader(commandList);

		commandList->SetGraphicsRootConstantBufferView(0, camera);
		commandList->SetGraphicsRootShaderResourceView(2, mat);
		commandList->SetGraphicsRootDescriptorTable(3, tex);
		commandList->SetGraphicsRootShaderResourceView(4, *boneIt);
		commandList->SetGraphicsRootShaderResourceView(5, *prevBoneIt);

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

	if(mRenderBB){
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
	}

}
