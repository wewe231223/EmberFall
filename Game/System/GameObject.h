#pragma once 
#include <vector>
#include <memory>
#include "../Game/Component/ComponentBase.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/Defines.h"

class ArcheType;

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
	template<typename T> 
	T& GetComponent() {

	}
private:
	ArcheType* mArcheType;
	size_t mArcheTypeIndex{ 0xFFFF'FFFF'FFFF'FFFF };
};


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
		template<typename T> 
		void Initialize() {
			CrashExp(mData == nullptr, "Can not be double Initialize");
			mElementSize = sizeof(T);
			mCapacity = 10;
			mData = std::make_unique<ComponentBase[]>(mCapacity);
		}

		void CheckReallocate(size_t current); 

		template<typename T> 
		T* Data() {
			CrashExp(mData != nullptr, "Data is nullptr");
			return static_cast<T>(mData.get());
		}

	private:
		size_t mType{ 0xFFFF'FFFF'FFFF'FFFF };
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

private:
	template<typename T> 
	void InitArcheType() {
		auto& back = mComponents.back();
		back.Initialize<T>();
	}

private:
	PartIdentifier mPartIdentifier{};

	std::vector<Container> mComponents{};
	size_t mCurrent{ 0 };
};
