#pragma once

#include "Script.h"

class CorruptedGemScript : public Script {
public:
    CorruptedGemScript(std::shared_ptr<GameObject> owner);
    virtual ~CorruptedGemScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    void OnDestroy(struct GemDestroyEvent* event);

private:
    float mDesytoyingTime{ 5.0f };
    SimpleMath::Vector3 mOriginColor{ };
};

