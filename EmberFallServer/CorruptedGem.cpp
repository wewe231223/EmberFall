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

    owner->mSpec.interactable = true;
    owner->mSpec.entity = Packets::EntityType_CORRUPTED_GEM;
    owner->mSpec.active = true;
    owner->mSpec.hp = 100.0f;
}

void CorruptedGemScript::Update(const float deltaTime) { }

void CorruptedGemScript::LateUpdate(const float deltaTime) { 
    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    if (false == owner->mSpec.active) {
        auto packetRemove = FbsPacketFactory::ObjectRemoveSC(owner->GetId());
        owner->StorePacket(packetRemove);

        gServerFrame->AddTimerEvent(owner->GetId(), owner->GetMyRoomIdx(), SysClock::now(), TimerEventType::REMOVE_NPC);
    }
}

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

    mDestroyingTime += event->elapsedTime;
    if (mDestroyingTime > DESTROYING_TIME) {
        auto ownerRoom = owner->GetMyRoomIdx();
        auto eventDestroyed = GameEventFactory::GetEvent<GemDestroyed>(owner->GetId(), event->sender);
        auto obj = gGameRoomManager->GetRoom(ownerRoom)->GetStage().GetObjectFromId(event->sender);
        if (nullptr != obj) {
            obj->DispatchGameEvent(eventDestroyed);
        }

        owner->mSpec.active = false;
    }
}

void CorruptedGemScript::CancelDestroying() {
    mDestroyingTime = 0.0f;
}
