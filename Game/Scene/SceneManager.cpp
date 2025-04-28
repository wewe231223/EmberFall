#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc) {

	mScenes[static_cast<size_t>(SceneType::TERRAIN)] = std::make_unique<TerrainScene>(renderMgr, mainCameraBufferLocation);
	mScenes[static_cast<size_t>(SceneType::LOADING)] = std::make_unique<LoadingScene>(renderMgr);


	mSceneFeatureType[static_cast<size_t>(SceneType::LOADING)] = std::make_tuple(false, false);
	mSceneFeatureType[static_cast<size_t>(SceneType::TERRAIN)] = std::make_tuple(true, true);

	mCurrentScene = mScenes[static_cast<size_t>(SceneType::LOADING)].get();

	gClientCore->Init();
	auto res = gClientCore->Start("127.0.0.1", 7777);
	if (false == res) {
		DebugBreak();
		Crash(false);
	}

	mLoadingThread = std::thread([this, device, loadCommandList, initLoadFunc]() {
		initLoadFunc();
		mScenes[static_cast<size_t>(SceneType::TERRAIN)]->Init(device, loadCommandList);
		mLoaded.store(true);
		});


}

SceneManager::~SceneManager() {
	if (mLoadingThread.joinable()) {
		mLoadingThread.join(); 
	}
}

std::tuple<bool, bool> SceneManager::GetCurrentSceneFeatureType() {
	return mSceneFeatureType[static_cast<size_t>(mCurrentSceneType)];
}

bool SceneManager::CheckLoaded() {
	auto loaded = mLoaded.load();

	if (loaded) {
		mLoadingThread.join();
		
		auto packet = FbsPacketFactory::PlayerEnterInGame(gClientCore->GetSessionId());
		gClientCore->Send(packet); 

		mCurrentScene = mScenes[static_cast<size_t>(SceneType::TERRAIN)].get();
		mCurrentSceneType = SceneType::TERRAIN;
		mLoaded.store(false);
	}

	return loaded;
}

void SceneManager::Update() {
	if (mCurrentScene) {
		mCurrentScene->ProcessNetwork();
		mCurrentScene->Update();
		mCurrentScene->SendNetwork(); 
	}
}