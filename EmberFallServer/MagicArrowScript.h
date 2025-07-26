#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ArrwoScript.h
// 
// 2025 - 02 - 10 : ArrwoScript
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Script.h"

class MagicArrowScript : public Script {
public:
    MagicArrowScript(std::shared_ptr<GameObject> owner, const SimpleMath::Vector3& pos,
        const SimpleMath::Vector3& dir);
    virtual ~MagicArrowScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;
};