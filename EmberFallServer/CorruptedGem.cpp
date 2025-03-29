#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"

#include "GameEventManager.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::CORRUPTED_GEM } { 
    owner->SetEntityType(EntityType::CORRUPTED_GEM);
}

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
    auto owner = GetOwner();
    if (event->holdTime > mDesytoyingTime) {
        auto gameEvent = std::make_shared<GemDestroyed>();
        gameEvent->sender = event->receiver;
        gameEvent->receiver = event->sender;
        gameEvent->type = GameEventType::DESTROY_GEM_COMPLETE;

        gEventManager->PushEvent(gameEvent);

        owner->SetActive(false);
    }
}
