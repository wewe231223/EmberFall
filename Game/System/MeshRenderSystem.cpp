#include "pch.h"
#include "MeshRenderSystem.h"
#include "../Game/Component/MeshRenderer.h"
#include "../Game/Component/Transform.h"

MeshRenderSystem::MeshRenderSystem(MeshRenderManager* renderManager) : mMeshRenderManager(renderManager){
	mPartIdentifier.Set<MeshRenderer>();
	mPartIdentifier.Set<Transform>();
}

MeshRenderSystem::~MeshRenderSystem() {

}

const PartIdentifier& MeshRenderSystem::GetPartIdentifier() const {
	return mPartIdentifier;
}

void MeshRenderSystem::Update(ArcheType* archeType) {

}
