#pragma once

#include "Script.h"
#include "ViewList.h"
#include "Inventory.h"

class Input;
class IServerGameScene;

class BossPlayerScript : public Script {
public:
    BossPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input);
    virtual ~BossPlayerScript();

public:
    void ResetGameScene(std::shared_ptr<IServerGameScene> gameScene);
    std::shared_ptr<IServerGameScene> GetCurrentScene() const;

    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;

    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;
    
private:
    void CheckAndMove(const float deltaTime);
    void CheckAndJump(const float deltaTime);

private:
    bool mInteraction{ false };
    ViewList mViewList;
    Inventory mInventory{ };

    std::shared_ptr<Input> mInput{ };
    std::shared_ptr<IServerGameScene> mGameScene{ };
    std::shared_ptr<GameObject> mInteractionTrigger{ };
};