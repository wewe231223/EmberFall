#pragma once 
#include "../Renderer/Render/Canvas.h"
#include "../Game/UI/SpriteImage.h"
#include "../Game/UI/Image.h"
#include "../Renderer/Manager/RenderManager.h"
#include "../ServerLib/PacketHandler.h"


class LoadingScene : public IScene {
public:
	LoadingScene(std::shared_ptr<RenderManager> renderMgr);
	~LoadingScene();
public:
	virtual void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	virtual void ProcessNetwork();
	virtual void Update();
	virtual void SendNetwork();
	virtual void Exit();
private:
	SpriteImage mLoading{};
	Image mBackground{};
	Image mLoadingWord{};
};