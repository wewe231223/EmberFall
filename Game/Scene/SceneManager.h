#pragma once 
#include "../Utility/Defines.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Manager/RenderManager.h"
#include <thread>

enum class SceneType : BYTE {
	TITLE,
	LOBBY,
	TERRAIN,
	FINAL,
	FINISH,
	LOADING,
	END,
};

class SceneManager {
public:
	SceneManager();
	~SceneManager();

public:
	SceneFeatureType GetCurrentSceneFeatureType();

	void Init(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc);
	bool CheckLoaded();
	void Update(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList);

	void ChangeSceneTo(SceneType nextScene);

private:
	std::array<std::shared_ptr<IScene>, static_cast<size_t>(SceneType::END)> mScenes{};
	std::array<SceneFeatureType, static_cast<size_t>(SceneType::END)> mSceneFeatureType{};

	IScene* mCurrentScene = nullptr;
	IScene* mNextScene = nullptr;

	SceneType mCurrentSceneType = SceneType::LOADING;
	SceneType mNextSceneType = SceneType::LOADING;

	std::atomic<bool> mLoaded{ false };
	std::thread mLoadingThread{};

	bool mAdvance = false;

	std::shared_ptr<RenderManager> mRenderManager{ nullptr };
	DefaultBufferCPUIterator mMainCameraBufferLocation{};
};
