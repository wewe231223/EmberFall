#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner } { 
}

CorruptedGemScript::~CorruptedGemScript() { }

void CorruptedGemScript::Update(const float deltaTime) { }

void CorruptedGemScript::LateUpdate(const float deltaTime) { }

void CorruptedGemScript::OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

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
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Pass Event My: {}, Taget: {}", GetOwner()->GetId(), event->target);
        return;
    }

    auto owner = GetOwner();
    mOriginColor = GetOwner()->GetColor();
    owner->SetColor(SimpleMath::Vector3{ event->holdTime / mDesytoyingTime });
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "SetColor {}", event->holdTime / mDesytoyingTime);
    if (event->holdTime > mDesytoyingTime) {
        owner->SetActive(false);
    }
}
