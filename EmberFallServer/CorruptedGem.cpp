#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"

#include "GameEventManager.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::CORRUPTED_GEM, ScriptType::CORRUPTED_GEM } { 
    owner->GetPhysics()->mFactor.mass = 10000.0f;
    owner->mSpec.entity = Packets::EntityType_CORRUPTED_GEM;
    owner->mSpec.active = true;
}

CorruptedGemScript::~CorruptedGemScript() { }

void CorruptedGemScript::Init() { }

void CorruptedGemScript::Update(const float deltaTime) { }

void CorruptedGemScript::LateUpdate(const float deltaTime) { }

void CorruptedGemScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void CorruptedGemScript::OnCollisionTerrain(const float height) { }

void CorruptedGemScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::DESTROY_GEM_EVENT:
        OnDestroy(reinterpret_cast<GemDestroyStart*>(event));
        break;

    default:
        break;
    }
}

void CorruptedGemScript::OnDestroy(GemDestroyStart* event) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (event->holdTime > mDesytoyingTime) {
        gEventManager->PushEvent<GemDestroyed>(
            event->receiver,
            event->sender
        );

        owner->mSpec.active = false;
    }
}
