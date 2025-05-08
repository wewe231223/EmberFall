#pragma once

#include "PlayerScript.h"

class BossPlayerScript : public PlayerScript {
public:
    BossPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input);
    virtual ~BossPlayerScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DoInteraction(const std::shared_ptr<GameObject>& target) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;
    
private:
    void CheckAndMove(const float deltaTime);
    void CheckAndJump(const float deltaTime);
};