#pragma once 
#include "../Game/System/System.h"
#include "../Game/System/GameObject.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/DirectXInclude.h"
#include "../Renderer/Manager/MeshRenderManager.h"

class MeshRenderSystem : public ISystem {
public:
	MeshRenderSystem(MeshRenderManager* renderManager); 
	virtual ~MeshRenderSystem(); 
public:
	virtual const PartIdentifier& GetPartIdentifier() const override;
	virtual void Update(ArcheType*) override;
private:
	PartIdentifier mPartIdentifier{};
	MeshRenderManager* mMeshRenderManager;
	
};