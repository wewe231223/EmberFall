#pragma once 
#include <vector>
#include <memory>
#include "../Game/Component/ComponentBase.h"

template <typename T>
concept HasIndex = requires {
	{ T::index } -> std::convertible_to<size_t>;
};

class GameObject {
public:
	GameObject() = default;
	~GameObject() = default;

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;
public:
	template<typename T> 
		requires std::is_base_of<ComponentBase, T>::value and HasIndex<T>
	T* GetComponent() {
		return static_cast<T*>(m_Components[T::index].get());
	}

	template<typename T, typename... Args>
		requires std::is_base_of<ComponentBase, T>::value and HasIndex<T>
	void AddComponent(Args&&... args) {
		m_Components[T::index] = std::make_shared<T>(std::forward<Args>(args)...);
	}
private:
	std::vector<std::shared_ptr<ComponentBase>> m_Components{ nullptr };
};
