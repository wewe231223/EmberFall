#pragma once 
#include <bitset>
#include "../Utility/Defines.h"
#include "../Utility/Crash.h"

using ComponentType = std::bitset<64>;

class PartIdentifier {
public:
	PartIdentifier() = default;

	template<typename... Types> 
	PartIdentifier() {
		((mParts.set(Types::index)), ...);
	}

	PartIdentifier(const PartIdentifier& other) {
		mParts = other.mParts;
	}

	PartIdentifier& operator=(const PartIdentifier& other) {
		mParts = other.mParts;
		return *this;
	}

	PartIdentifier(PartIdentifier&& other) noexcept {
		mParts = std::move(other.mParts);
	}

	PartIdentifier& operator=(PartIdentifier&& other) noexcept {
		mParts = std::move(other.mParts);
		return *this;
	}

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

	bool Test(size_t index) const {
		CrashExp(index < mParts.size(), "Index out of range");
		return mParts.test(index);
	}
private:
	ComponentType mParts{};
};
