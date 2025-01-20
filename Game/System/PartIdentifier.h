#pragma once 
#include <bitset>
#include "../Utility/Defines.h"
#include "../Utility/Crash.h"

class PartIdentifier {
public:
	PartIdentifier() = default;
public:
	bool operator==(const PartIdentifier& other) const {
		return PartIdentifier::Test(other);
	}
public:
	template<typename T> requires HasIndex<T>
	void Set() {
		CrashExp(T::index < mParts.size(), "Index out of range");
		mParts.set(T::index);
	}

	bool Test(const PartIdentifier& other) const {
		return mParts == other.mParts;
	}
private:
	std::bitset<64> mParts{};
};