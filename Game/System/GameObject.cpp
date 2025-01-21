#include "pch.h"
#include "GameObject.h"

void ArcheType::Container::CheckReallocate(size_t current) {
	CrashExp(mData != nullptr, "Data is nullptr");

	if (mCapacity <= current) {
		mCapacity *= 2;
		auto temp = std::make_unique<ComponentBase[]>(mCapacity);
		memcpy(temp.get(), mData.get(), mElementSize * mCapacity);
		mData = std::move(temp);
	}
}

GameObject ArcheType::CreateGameObject() {	
	mCurrent++;
	for (auto& componentArr : mComponents) {
		componentArr.CheckReallocate(mCurrent);
	}

	GameObject result{};
	result.mArcheTypeIndex = mCurrent;
	result.mArcheType = this;

	return result;
}
