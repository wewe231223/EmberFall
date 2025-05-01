#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc) {

	mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_unique<TerrainScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_unique<LoadingScene>(renderMgr);
	mScenes[static_cast<size_t>(SceneType::LOBBY)] = std::make_unique<LobbyScene>(renderMgr, mainCameraBufferLocation);

	mSceneFeatureType[static_cast<size_t>(SceneType::LOADING)] = std::make_tuple(false, false);
	mSceneFeatureType[static_cast<size_t>(SceneType::LOBBY)] = std::make_tuple(false, false);
	mSceneFeatureType[static_cast<size_t>(SceneType::TERRAIN)] = std::make_tuple(true, true);


	mSceneGraph[0] = std::make_pair(mScenes[static_cast<size_t>(SceneType::LOADING)].get(), mScenes[static_cast<size_t>(SceneType::LOBBY)].get());
	mSceneGraph[1] = std::make_pair(mScenes[static_cast<size_t>(SceneType::LOBBY)].get(), mScenes[static_cast<size_t>(SceneType::TERRAIN)].get());
	mSceneGraph[2] = std::make_pair(mScenes[static_cast<size_t>(SceneType::TERRAIN)].get(), mScenes[static_cast<size_t>(SceneType::LOADING)].get());

	mCurrentSceneNode = mSceneGraph.begin(); 
	mCurrentSceneFeatureType = mSceneFeatureType.begin();


	gClientCore->Init();
	auto res = gClientCore->Start("192.168.66.231", 7777);
	if (false == res) {
		DebugBreak();
		Crash(false);
	}

	mLoadingThread = std::thread([this, device, loadCommandList, initLoadFunc]() {
		initLoadFunc();
		mCurrentSceneNode->second->Init(device, loadCommandList);
		mLoaded.store(true);
		});

	 
}

SceneManager::~SceneManager() {
	if (mLoadingThread.joinable()) {
		mLoadingThread.join(); 
	}
}

SceneFeatureType SceneManager::GetCurrentSceneFeatureType() {
	return *mCurrentSceneFeatureType; 
}

bool SceneManager::CheckLoaded() {
	auto loaded = mLoaded.load();

	if (loaded) {
		mLoadingThread.join();

		mCurrentSceneFeatureType++;
		mCurrentSceneNode++;

		mLoaded.store(false);
	}

	return loaded;
}

void SceneManager::Update(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList) {

	if (mAdvance and not mLoadingThread.joinable()) {
		mCurrentSceneNode->first->Exit();
		mLoadingThread = std::thread([this, device, loadCommandList]() {
			mCurrentSceneNode->second->Init(device, loadCommandList);
			mLoaded.store(true);
			});
		mAdvance = false;
	}

	if (mLoadingThread.joinable()) {
		auto& loadingScene = mScenes[static_cast<size_t>(SceneType::LOADING)];
		loadingScene->ProcessNetwork();
		loadingScene->Update();
		loadingScene->SendNetwork(); 
		return;
	}

	if (mCurrentSceneNode->first != nullptr) {
		mCurrentSceneNode->first->ProcessNetwork();
		mCurrentSceneNode->first->Update();
		mCurrentSceneNode->first->SendNetwork(); 
	}
}

void SceneManager::AdvanceScene() {
	mAdvance = true;
}
