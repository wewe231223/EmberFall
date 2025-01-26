#pragma once 
#include "../Game/System/PartIdentifier.h"
#include "../Game/System/GameObject.h"

//
// 1. runtime 에는 새로운 ArcheType 이 만들어지거나, 어느 하나의 GameObject 의 Archetype 이라도 Archetype 이 변경 될 수 없다. 
// 2. 1 로 인하여 게임 루프가 시작되기 전에, 모든 GameObject( ArcheType ) 이 적용될 System 을 결정 할 수 있다. 
//

struct ISystem abstract {
	virtual ~ISystem() = default;
	virtual const PartIdentifier& GetPartIdentifier() const = 0;
	virtual void Subscribe(ArcheType* archeType) = 0;
	virtual void Update() = 0;
};