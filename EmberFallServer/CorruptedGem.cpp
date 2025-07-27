#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"
#include "ServerFrame.h"

#include "GameRoom.h"
#include "ObjectManager.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner, ObjectTag::CORRUPTED_GEM, ScriptType::CORRUPTED_GEM } { }

CorruptedGemScript::~CorruptedGemScript() { }

void CorruptedGemScript::Init() { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.moveable = false;
    spec.interactable = true;
    spec.animated = false;
    spec.entity = Packets::EntityType_CORRUPTED_GEM;
    spec.hp = 100000.0f;
}

void CorruptedGemScript::Update(const float deltaTime) { }

void CorruptedGemScript::LateUpdate(const float deltaTime) { }

void CorruptedGemScript::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void CorruptedGemScript::OnCollisionTerrain(const float height) { }

void CorruptedGemScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::DESTROY_GEM_EVENT:
        OnDestroy(reinterpret_cast<DestroyingGemEvent*>(event));
        break;
        
    case GameEventType::DESTTOY_GEM_CANCEL:
        CancelDestroying();
        break;

    default:
        break;
    }
}

void CorruptedGemScript::OnDestroy(DestroyingGemEvent* event) {
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (false == owner->mSpec.active) {
        return;
    }

    mDestroyingTime += event->elapsedTime;
    if (mDestroyingTime > DESTROYING_TIME) {
        auto ownerRoom = owner->GetMyRoomIdx();
        auto eventDestroyed = GameEventFactory::GetEvent<GemDestroyed>(owner->GetId(), event->sender, owner->GetPosition());
        auto obj = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(event->sender);
        if (nullptr != obj) {
            obj->DispatchGameEvent(eventDestroyed);
        }

        bool expectedRemoved = false;
        if (false == owner->mRemoved.compare_exchange_strong(expectedRemoved, true)) {
            return;
        }
        gServerFrame->AddTimerEvent(owner->GetId(), EXECUTE_IMMEDIATE, IoType::REMOVE_NPC, owner->GetMyRoomIdx());
    }
}

void CorruptedGemScript::CancelDestroying() {
    mDestroyingTime = 0.0f;
}
