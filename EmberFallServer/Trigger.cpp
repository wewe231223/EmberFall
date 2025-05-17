#include "pch.h"
#include "Trigger.h"
#include "Collider.h"
#include "GameTimer.h"
#include "GameEventFactory.h"
#include "GameObject.h"
#include "ServerFrame.h"

Trigger::Trigger(std::shared_ptr<GameObject> owner, float lifeTime)
    : Script{ owner, ObjectTag::TRIGGER, ScriptType::TRIGGER }, mLifeTime{ lifeTime } {  }

Trigger::~Trigger() { }

std::unordered_set<NetworkObjectIdType>& Trigger::GetObjects() {
    return mInTriggerObjects;
}

void Trigger::Init() {
    if (MathUtil::IsMax(mLifeTime)) {
        return;
    }

    auto owner = GetOwner();
    if (nullptr == owner) {
        return;
    }

    auto& spec = owner->mSpec;
    spec.active = true;
    spec.moveable = false;
    spec.interactable = false;
    spec.animated = false;
    spec.entity = Packets::EntityType_ENV;

    auto id = owner->GetId();
    auto executeTime = SysClock::now() + std::chrono::milliseconds{ static_cast<long long>(mLifeTime * 1000.0f) };
    gServerFrame->AddTimerEvent(owner->GetMyRoomIdx(), id, executeTime, TimerEventType::REMOVE_TRIGGER);
}

void Trigger::Update(const float deltaTime) { }

void Trigger::LateUpdate(const float deltaTime) { 
    mInTriggerObjects.clear();
}

void Trigger::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (ObjectTag::TRIGGER == opponent->GetTag()) {
        return;
    }

    mInTriggerObjects.insert(opponent->GetId());
}

void Trigger::OnCollisionTerrain(const float height) { }

void Trigger::DispatchGameEvent(GameEvent* event) { }
