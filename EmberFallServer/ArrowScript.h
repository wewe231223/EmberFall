#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CorruptedGem.h
// 
// 2025 - 02 - 10 : CorruptedGemScript
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Script.h"

class ArrowScript : public Script {
public:
    ArrowScript(std::shared_ptr<GameObject> owner, const SimpleMath::Vector3& pos,
        const SimpleMath::Vector3& dir, GameUnits::GameUnit<GameUnits::StandardSpeed> speed);
    virtual ~ArrowScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;
};