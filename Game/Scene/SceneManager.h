#pragma once 
#include "../Utility/Defines.h"
#include "../Scene/LoadingScene.h"
#include "../Scene/TerrainScene.h"
#include "../Scene/LobbyScene.h"

// 어느 한 Scene 을 로딩하면, 다음 씬 로드, 
// 현재 Scene 이 다음 Scene 으로 전환 되어야 할 때, 
// 1. 다음 Scene 이 모두 로딩 되었다면 ( LoadingThread 가 joinable 이라면 ), -> 전환. 
// 2. 다음 Scene 이 아직 로딩 중이라면 ( LoagingThread 가 joinable 이 아니라면 ), -> LoadingScene 으로 전환. 
// 
// 맨 처음 Renderer 의 초기화  ( 전체 텍스쳐 로딩 ) 는 TitleScene 이 있다면, LobbyScene 로딩과 함께 진행, 아니라면, LoadingScene 을 보여주고, 로딩. 

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
	SceneManager(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc);
	~SceneManager();

public:
	SceneFeatureType GetCurrentSceneFeatureType();

	bool CheckLoaded(); 
	void Update(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList);
 
	void AdvanceScene();
private:
	std::array<std::unique_ptr<IScene>, static_cast<size_t>(SceneType::END)> mScenes{};
	std::array<SceneFeatureType, static_cast<size_t>(SceneType::END)> mSceneFeatureType{};
	std::array<SceneFeatureType, static_cast<size_t>(SceneType::END)>::iterator mCurrentSceneFeatureType{};

	std::array<std::pair<IScene*, IScene*>, 3> mSceneGraph{};
	std::array<std::pair<IScene*, IScene*>, 3>::iterator mCurrentSceneNode{};
	
	std::atomic<bool> mLoaded{ false };
	std::thread mLoadingThread{};

	bool mAdvance{ false }; 
};