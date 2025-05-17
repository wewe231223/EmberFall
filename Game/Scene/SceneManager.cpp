#include "pch.h"
#include "SceneManager.h"
#include "../Scene/LoadingScene.h"
#include "../Scene/TerrainScene.h"
#include "../Scene/LobbyScene.h"
#include "../resource.h"
#include "../Renderer/Core/Console.h"

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {
	if (mLoadingThread.joinable()) {
		mLoadingThread.join();
	}
}

void SceneManager::Init(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc) {
	mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_shared<TerrainScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_shared<LoadingScene>(renderMgr);
	mScenes[static_cast<size_t>(SceneType::LOBBY)] = std::make_shared<LobbyScene>(renderMgr, mainCameraBufferLocation);

	mSceneFeatureType[static_cast<size_t>(SceneType::LOADING)]	= { false, false, false }; 
	mSceneFeatureType[static_cast<size_t>(SceneType::LOBBY)]	= { false, false, true }; 
	mSceneFeatureType[static_cast<size_t>(SceneType::TERRAIN)]	= { true, true, true, true }; 

	mCurrentSceneType = SceneType::LOADING;
	mCurrentScene = mScenes[static_cast<size_t>(SceneType::LOADING)].get();

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
}

bool SceneManager::CheckLoaded() {
	auto loaded = mLoaded.load();

	if (loaded) {
		if (mLoadingThread.joinable()) {
			mLoadingThread.join();
		}
		mCurrentSceneType = mNextSceneType;
		mCurrentScene = mNextScene;

		mRenderManager->GetFeatureManager().SetFeature(mSceneFeatureType[static_cast<size_t>(mCurrentSceneType)]);
		mRenderManager->GetFeatureManager().SetFixedFeatures(mSceneFeatureType[static_cast<size_t>(mCurrentSceneType)]);
		mLoaded.store(false);
	}
	return loaded; 
}

void SceneManager::Update(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList) {
	if (mAdvance && !mLoadingThread.joinable()) {
		if (mCurrentScene) mCurrentScene->Exit();

		switch (mNextSceneType) {
		case SceneType::TITLE:
			break;
		case SceneType::LOBBY:
			mScenes[static_cast<size_t>(SceneType::LOBBY)] = std::make_unique<LobbyScene>(mRenderManager, mMainCameraBufferLocation);
			Console.Log("Lobby Scene 이 로드되었습니다.", LogType::Warning); 
			break;
		case SceneType::TERRAIN:
			mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_unique<TerrainScene>(mRenderManager, mMainCameraBufferLocation);
			Console.Log("Terrain Scene 이 로드되었습니다.", LogType::Warning);
			break;
		case SceneType::FINAL:
			break;
		case SceneType::FINISH:
			break;
		case SceneType::LOADING:
			mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_unique<LoadingScene>(mRenderManager);
			Console.Log("Loading Scene 이 로드되었습니다.", LogType::Warning);
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
