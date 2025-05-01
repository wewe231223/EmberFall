#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CorruptedGem.h
// 
// 2025 - 02 - 10 : CorruptedGemScript
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Script.h"

class CorruptedGemScript : public Script {
private:
    inline static constexpr float DESTROYING_TIME{ 3.0f };

public:
    CorruptedGemScript(std::shared_ptr<GameObject> owner);
    virtual ~CorruptedGemScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override; 
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    void OnDestroy(struct DestroyingGemEvent* event);
    void CancelDestroying();

private:
    float mDestroyingTime{ 0.0f };
};

