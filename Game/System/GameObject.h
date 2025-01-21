#pragma once 
#include <vector>
#include <memory>
#include "../Game/Component/ComponentBase.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/Defines.h"

class ArcheType;

class GameObject {
public:
	GameObject() = default;
	~GameObject() = default;

	GameObject(const GameObject&);
	GameObject& operator=(const GameObject&);

	GameObject(GameObject&&) noexcept;
	GameObject& operator=(GameObject&&) noexcept;
public:

};



class ArcheType {
	using Cont = std::vector<std::pair<size_t, std::unique_ptr<ComponentBase[]>>>;

public:
	ArcheType() = default;
	~ArcheType() = default;

	ArcheType(const ArcheType&);
	ArcheType& operator=(const ArcheType&);
	
	ArcheType(ArcheType&&) noexcept = default;
	ArcheType& operator=(ArcheType&&) noexcept = default;
public:
	GameObject CreateGameObject(); 
private:
	PartIdentifier mPartIdentifier{};

	Cont mComponents{};

	size_t mCurrent{ 0 };
	size_t mComponentArraySize{ 10 };
};