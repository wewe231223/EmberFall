#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner } { }

CorruptedGemScript::~CorruptedGemScript() { }

void CorruptedGemScript::Init() { }

void CorruptedGemScript::Update(const float deltaTime) { }

void CorruptedGemScript::LateUpdate(const float deltaTime) { }

void CorruptedGemScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent) { }

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
    if (event->target != GetOwner()->GetId()) {
        return;
    }

    auto owner = GetOwner();
    mOriginColor = GetOwner()->GetColor();
    owner->SetColor(SimpleMath::Vector3{ event->holdTime / mDesytoyingTime });
    if (event->holdTime > mDesytoyingTime) {
        owner->SetActive(false);
    }
}
