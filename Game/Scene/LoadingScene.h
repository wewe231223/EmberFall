#pragma once 
#include "../Renderer/Render/Canvas.h"
#include "../Game/UI/Loading.h"
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Renderer/Manager/ParticleManager.h"
#include "../ServerLib/PacketHandler.h"


class LoadingScene : public IScene {
public:
	LoadingScene(std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>, std::shared_ptr<Canvas>> managers);
	~LoadingScene();
public:
	virtual void Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	virtual void ProcessNetwork();
	virtual void Update();
	virtual void SendNetwork();
private:
	Loading mLoading{};
};