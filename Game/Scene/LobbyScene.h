#pragma once 
#include "../Renderer/Manager/RenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../Game/GameObject/GameObject.h"
#include "../ServerLib/PacketHandler.h"


class LobbyScene : public IScene {
public:
	LobbyScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation);
	virtual ~LobbyScene();
public:
	virtual void Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) override;
	virtual void ProcessNetwork() override;
	virtual void Update() override;
	virtual void SendNetwork() override;
private:
	void BuildMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);
	void BuildMaterial();
	void BuildShader(ComPtr<ID3D12Device> device);

private:
	std::shared_ptr<RenderManager> mRenderManager{};

	Camera mCamera{};

	GameObject mSkyBox{};

	std::unordered_map<std::string, std::unique_ptr<Mesh>> mMeshMap{};
	std::unordered_map<std::string, std::unique_ptr<GraphicsShaderBase>> mShaderMap{};
	std::unordered_map<std::string, AnimationLoader> mAnimationMap{};
};