#pragma once  
#include <array>
#include "../Renderer/Manager/RenderManager.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Game/System/Input.h"
#include "../Game/System/Timer.h"
#include "../System/Sound.h"
#include "../Game/GameObject/GameObject.h"
#include "../Game/Scene/Camera.h"
#include "../Game/GameObject/Animator.h"
#include "../Game/GameObject/EquipmentObject.h"
#include "../MeshLoader/Loader/TerrainLoader.h"
#include "../Game/Scene/Player.h"
#include "../ServerLib/PacketHandler.h"
#include "../Utility/IntervalTimer.h"
#include "../UI/Inventory.h"
#include "../UI/HealthBar.h"
#include "../UI/Profile.h"
#include "../Game/GameObject/TerrainObject.h"
#include "../External/Include/absl/container/flat_hash_map.h"
#include "../Game/Scene/MaterialLoader.h"
#include "../Game/UI/Entry.h"

class LogInScene : public IScene {
public:
	LogInScene(std::shared_ptr<RenderManager> renderMgr, DefaultBufferCPUIterator mainCamLocation);
	virtual ~LogInScene();

public:
	virtual void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) override;

	virtual void ProcessNetwork();
	virtual void Update();
	virtual void SendNetwork();
	virtual void Exit();
private:

	Entry mIDEntry{};
	Entry mPWEntry{}; 

};
