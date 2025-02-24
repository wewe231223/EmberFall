#pragma once

#include "Script.h"

class CorruptedGemScript : public Script {
public:
    CorruptedGemScript(std::shared_ptr<GameObject> owner);
    virtual ~CorruptedGemScript();

public:
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;

    virtual void DispatchGameEvent(class GameEvent* event) override;

private:
    SimpleMath::Vector3 mOriginColor{ };
};

