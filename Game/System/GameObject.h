#pragma once 
#include <memory>
#include <span>
#include <numeric>
#include <unordered_map>
#include "../Game/Component/ComponentBase.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/Defines.h"

class GameObject;

class ArcheType {
	class Container {
	public:
		Container() = default;
		~Container() = default;

		Container(const Container&) = delete;
		Container& operator=(const Container&) = delete;

		Container(Container&&) noexcept = default;
		Container& operator=(Container&&) noexcept = default;
	public:
		bool operator==(ComponentType type) const;
	public:
		template<typename T> 
		void Initialize() {
			CrashExp(mData == nullptr, "Can not be double Initialize");
			mElementSize = sizeof(T);
			mCapacity = 10;
			mData = std::make_unique<T[]>(mCapacity);
		}

		void CheckReallocate(size_t current); 

		template<typename T> 
		std::span<T> GetComponentArr(size_t current) {
			CrashExp(mCapacity >= current, "Out of range");
			return std::span<T>(reinterpret_cast<T*>(mData.get()), current);
		}

	private:
		ComponentType mType{};
		size_t mElementSize{ 0 };
		size_t mCapacity{ 0 };
		std::unique_ptr<ComponentBase[]> mData{ nullptr };
	};
public:
	ArcheType() = default;
	~ArcheType() = default;

	ArcheType(const ArcheType&) = delete;
	ArcheType& operator=(const ArcheType&) = delete;
	
	ArcheType(ArcheType&&) noexcept = default;
	ArcheType& operator=(ArcheType&&) noexcept = default;
public:
	template<typename ...ComponentTypes> 
	void Initialize() {
		(ArcheType::InitArcheType<ComponentTypes>(), ...);
	}

	GameObject CreateGameObject();

	template<typename T> 
	std::span<T> GetComponentArr() {
		return mComponents[T::index].GetComponentArr<T>(mCurrent);
	}

	bool CheckCompatibility(const PartIdentifier& partIdentifier) const {
		return mPartIdentifier == partIdentifier;
	}

private:
	template<typename T> requires HasIndex<T> 
	void InitArcheType() {
		mComponents[T::index].Initialize<T>();
	}

private:
	PartIdentifier mPartIdentifier{};
	std::unordered_map<ComponentType, Container> mComponents{};
	size_t mCurrent{ 0 };
};


class GameObject {
	friend ArcheType;
public:
	GameObject() = default;
	~GameObject() = default;

	GameObject(const GameObject&) = default;
	GameObject& operator=(const GameObject&) = default;

	GameObject(GameObject&&) noexcept = default;
	GameObject& operator=(GameObject&&) noexcept = default;
public:
	template<typename T> requires HasIndex<T> 
	T& GetComponent() {
		CrashExp(mArcheType != nullptr, "ArcheType is nullptr");
		return mArcheType->mComponents[T::index].GetComponentArr<T>(mArcheTypeIndex);
	}
private:
	ArcheType* mArcheType{ nullptr };
	size_t mArcheTypeIndex{ std::numeric_limits<size_t>::max() };
};
