#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::NONE } { }

CorruptedGemScript::~CorruptedGemScript() { }

void CorruptedGemScript::Init() { }

void CorruptedGemScript::Update(const float deltaTime) { }

void CorruptedGemScript::LateUpdate(const float deltaTime) { }

void CorruptedGemScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void CorruptedGemScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void CorruptedGemScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void CorruptedGemScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::DESTROY_GEM_EVENT:
        OnDestroy(reinterpret_cast<GemDestroyEvent*>(event));
        break;

    default:
        break;
    }
}

void CorruptedGemScript::OnDestroy(GemDestroyEvent* event) {
    if (event->receiver != GetOwner()->GetId()) {
        return;
    }

    auto owner = GetOwner();
    if (event->holdTime > mDesytoyingTime) {
        owner->SetActive(false);
    }
}
