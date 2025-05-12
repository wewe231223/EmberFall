#include "pch.h"
#include "SceneManager.h"
#include "../Scene/LoadingScene.h"
#include "../Scene/TerrainScene.h"
#include "../Scene/LobbyScene.h"

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {
	if (mLoadingThread.joinable()) {
		mLoadingThread.join();
	}
}

SceneFeatureType SceneManager::GetCurrentSceneFeatureType() {
	return mSceneFeatureType[static_cast<size_t>(mCurrentSceneType)];
}

void SceneManager::Init(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc) {
	mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_shared<TerrainScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_shared<LoadingScene>(renderMgr);
	mScenes[static_cast<size_t>(SceneType::LOBBY)] = std::make_shared<LobbyScene>(renderMgr, mainCameraBufferLocation);

	mSceneFeatureType[static_cast<size_t>(SceneType::LOADING)] = std::make_tuple(false, false, false);
	mSceneFeatureType[static_cast<size_t>(SceneType::LOBBY)] = std::make_tuple(false, false, true);
	mSceneFeatureType[static_cast<size_t>(SceneType::TERRAIN)] = std::make_tuple(true, true, true);

	mCurrentSceneType = SceneType::LOADING;
	mCurrentScene = mScenes[static_cast<size_t>(SceneType::LOADING)].get();

	gClientCore->Init();
	if (!gClientCore->Start("183.101.111.244", 7777)) {
		DebugBreak();
		Crash(false);
	}

	mNextSceneType = SceneType::LOBBY;
	mNextScene = mScenes[static_cast<size_t>(SceneType::LOBBY)].get();

	mRenderManager = renderMgr;
	mMainCameraBufferLocation = mainCameraBufferLocation;

	mLoadingThread = std::thread([this, device, loadCommandList, initLoadFunc]() {
		initLoadFunc(); 
		mNextScene->Init(device, loadCommandList); 
		mLoaded.store(true);
		});
}

void SceneManager::ChangeSceneTo(SceneType nextScene) {
	mAdvance = true;
	mNextSceneType = nextScene;
	mNextScene = mScenes[static_cast<size_t>(nextScene)].get();
}

bool SceneManager::CheckLoaded() {
	auto loaded = mLoaded.load();

	if (loaded) {
		if (mLoadingThread.joinable()) {
			mLoadingThread.join();
		}
		mCurrentSceneType = mNextSceneType;
		mCurrentScene = mNextScene;
		mLoaded.store(false);
	}
	return loaded; 
}

void SceneManager::Update(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList) {
	if (mAdvance && !mLoadingThread.joinable()) {
		if (mCurrentScene) mCurrentScene->Exit();

		switch (mNextSceneType)
		{
		case SceneType::TITLE:
			break;
		case SceneType::LOBBY:
			mScenes[static_cast<size_t>(SceneType::LOBBY)] = std::make_unique<LobbyScene>(mRenderManager, mMainCameraBufferLocation);
			break;
		case SceneType::TERRAIN:
			mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_unique<TerrainScene>(mRenderManager, mMainCameraBufferLocation);
			break;
		case SceneType::FINAL:
			break;
		case SceneType::FINISH:
			break;
		case SceneType::LOADING:
			mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_unique<LoadingScene>(mRenderManager);
			break;
		default:
			break;
		}

		mNextScene = mScenes[static_cast<size_t>(mNextSceneType)].get();

		mLoadingThread = std::thread([this, device, loadCommandList]() {
			mNextScene->Init(device, loadCommandList);
			mLoaded.store(true);
			});

		mCurrentScene = mScenes[static_cast<size_t>(SceneType::LOADING)].get(); 
		mCurrentSceneType = SceneType::LOADING;
		mAdvance = false;
	}

	if (mCurrentScene) {
		mCurrentScene->ProcessNetwork();
		mCurrentScene->Update();
		mCurrentScene->SendNetwork();
	}
}
