#pragma once 
#include <vector>
#include <memory>
#include <any>
#include "../Game/System/PartIdentifier.h"
#include "../Game/Component/ComponentBase.h"

//class ArcheType {
//	using Cont = std::vector<std::pair<size_t, std::unique_ptr<ComponentBase[]>>>;
//public:
//	ArcheType() = default;
//	~ArcheType() = default;
//
//	ArcheType(const ArcheType&);
//	ArcheType& operator=(const ArcheType&);
//	
//	ArcheType(ArcheType&&) noexcept = default;
//	ArcheType& operator=(ArcheType&&) noexcept = default;
//public:
//	std::pair<ArcheType&, size_t> CreateComponents();
//private:
//	PartIdentifier mPartIdentifier{};
//
//	Cont mComponents{};
//
//	size_t mCurrent{ 0 };
//	size_t mComponentArraySize{ 10 };
//};
