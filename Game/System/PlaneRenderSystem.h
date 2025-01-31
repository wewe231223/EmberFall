#pragma once 
#include "../Game/System/System.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/DirectXInclude.h"
#include <vector>

// 오브젝트의 상수 버퍼를 작성하는 시스템 
// 프러스텀 컬링은 어디에서?? 

class PlaneRenderSystem : public ISystem {
public:
	PlaneRenderSystem();
	virtual ~PlaneRenderSystem(); 
public:
	virtual void Subscribe(ArcheType* archeType) override;
	virtual const PartIdentifier& GetPartIdentifier() const override;
	virtual void Update() override;
private:
	PartIdentifier mPartIdentifier{};
	std::vector<ArcheType*> mArcheTypes{};

	void* mModelContext{ nullptr };
};
