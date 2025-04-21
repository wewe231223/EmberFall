#pragma once 
#include "../Utility/Defines.h"
#include "../Scene/LoadingScene.h"
#include "../Scene/TerrainScene.h"

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
	SceneManager(std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>, std::shared_ptr<Canvas>> managers, DefaultBufferCPUIterator mainCameraBufferLocation, ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> loadCommandList, std::function<void()> initLoadFunc);
	~SceneManager();

public:
	std::tuple<bool, bool> GetCurrentSceneFeatureType();

	bool CheckLoaded(); 
	void Update();
private:
	std::array<std::unique_ptr<IScene>, static_cast<size_t>(SceneType::END)> mScenes{};
	std::array<std::tuple<bool, bool>, static_cast<size_t>(SceneType::END)> mSceneFeatureType{};

	std::array<std::pair<IScene*, IScene*>, 3> mSceneGraph{};
	
	SceneType mCurrentSceneType{ SceneType::LOADING };
	IScene* mCurrentScene{ nullptr };

	std::atomic<bool> mLoaded{ false };
	std::thread mLoadingThread{};

};