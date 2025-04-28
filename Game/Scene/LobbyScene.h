#pragma once 
#include "../Renderer/Manager/RenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../Game/GameObject/GameObject.h"
#include "../ServerLib/PacketHandler.h"


class LobbyScene : public IScene {
public:
	LobbyScene(std::shared_ptr<RenderManager> renderMgr);
	virtual ~LobbyScene();
public:
	virtual void Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) override;
	virtual void ProcessNetwork() override;
	virtual void Update() override;
	virtual void SendNetwork() override;
private:
	void BuildMesh(); 
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);

};