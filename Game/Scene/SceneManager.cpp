#include "SceneManager.h"
#include "pch.h"
#include "SceneManager.h"
#include "../Scene/LoadingScene.h"
#include "../Scene/LogInScene.h"
#include "../Scene/TerrainScene.h"
#include "../Scene/LobbyScene.h"
#include "../Scene/AreanaScene.h"
#include "../resource.h"
#include "../Renderer/Core/Console.h"

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {
	if (mLoadingThread.joinable()) {
		mLoadingThread.join();
	}
}

void SceneManager::Init(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc) {
	mScenes[static_cast<size_t>(SceneType::TITLE)] = std::make_shared<LogInScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_shared<TerrainScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_shared<LoadingScene>(renderMgr);
	mScenes[static_cast<size_t>(SceneType::LOBBY)] = std::make_shared<LobbyScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::FINAL)] = std::make_shared<ArenaScene>(renderMgr, mainCameraBufferLocation);

	mSceneFeatureType[static_cast<size_t>(SceneType::TITLE)] = { false, false, false, false, false, false, false };
	mSceneFeatureType[static_cast<size_t>(SceneType::LOADING)]	= { false, false, false, false, false, false, false }; 
	mSceneFeatureType[static_cast<size_t>(SceneType::LOBBY)]	= { false, false, true, false, false, false, false }; 
	mSceneFeatureType[static_cast<size_t>(SceneType::TERRAIN)]	= { true, true, true, true, true, false, true }; 
	mSceneFeatureType[static_cast<size_t>(SceneType::FINAL)]	= { true, true, true, true, true, false, false };

	mCurrentSceneType = SceneType::LOADING;
	mCurrentScene = mScenes[static_cast<size_t>(SceneType::LOADING)].get();

#ifdef START_WITH_TITLE
	mNextSceneType = SceneType::TITLE;
	mNextScene = mScenes[static_cast<size_t>(SceneType::TITLE)].get();
#else 
	mNextSceneType = SceneType::LOBBY;
	mNextScene = mScenes[static_cast<size_t>(SceneType::LOBBY)].get();
#endif 

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

	switch (mNextSceneType) {
	case SceneType::TITLE:
		Console.Log("Title Scene 으로 변경합니다.", LogType::Warning);
		break;
	case SceneType::LOBBY:
		Console.Log("Lobby Scene 으로 변경합니다.", LogType::Warning);
		break;
	case SceneType::TERRAIN:
		Console.Log("Terrain Scene 으로 변경합니다.", LogType::Warning);
		break;
	case SceneType::FINAL:
		Console.Log("Arena Scene 으로 변경합니다.", LogType::Warning);
		break;
	case SceneType::FINISH:
		break;
	case SceneType::LOADING:
		Console.Log("Loading Scene 으로 변경합니다.", LogType::Warning);
		break;
	default:
		break;
	}

}

UINT SceneManager::GetCurrentSceneIndex() const {
	switch (mCurrentSceneType) {
	case SceneType::TITLE:
		return 3; 
		break;
	case SceneType::LOBBY:
		return 3;
		break;
	case SceneType::TERRAIN:
		return 0;
		break;
	case SceneType::FINAL:
		return 1;
		break;
	case SceneType::FINISH:
		return 3;
		break;
	case SceneType::LOADING:
		return 3;
		break;
	case SceneType::END:
		return 3;
		break;
	default:
		return 3;
		break;
	}
	return 3;
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
			mScenes[static_cast<size_t>(SceneType::FINAL)] = std::make_unique<ArenaScene>(mRenderManager, mMainCameraBufferLocation);
			Console.Log("Arena Scene 이 로드되었습니다.", LogType::Warning);
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
