#pragma once 
#include <memory>

class SceneManager {
	friend class Renderer;
public:
	SceneManager() = default;
	~SceneManager() = default;
private:
	struct SceneManagerImpl;
	std::unique_ptr<SceneManagerImpl> mSceneManagerImpl{ nullptr };
};