#pragma once 
#include "../Game/System/GameObject.h"

class Scene {
public:
	Scene();
	~Scene();


public:
	template<typename... Types> 
	void CreateGameObject(const std::string& name) {
		ComponentType type{};
		((type.set(Types::index)), ...);
		mGameObjects[name] = mArchtypes[type].CreateGameObject();
	}
private:
	std::unordered_map<ComponentType, ArcheType, std::hash<ComponentType>> mArchtypes{};
	std::unordered_map<std::string, GameObject, std::hash<std::string>> mGameObjects{};
};

