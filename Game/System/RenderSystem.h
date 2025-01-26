#pragma once 
#include "../Game/System/System.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/DirectXInclude.h"
#include <vector>

class RenderSystem : public ISystem {
public:
	RenderSystem();
	virtual ~RenderSystem(); 
public:
	virtual void Subscribe(ArcheType* archeType) override;
	virtual const PartIdentifier& GetPartIdentifier() const override;
	virtual void Update() override;
private:
	PartIdentifier mPartIdentifier{};
	std::vector<ArcheType*> mArcheTypes{};


};
