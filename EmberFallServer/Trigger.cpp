#include "pch.h"
#include "Trigger.h"
#include "Collider.h"
#include "GameEventManager.h"
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

    auto id = owner->GetId();
    auto excuteTime = mLifeTime;
    gServerFrame->AddTimerEvent(id, SysClock::now() + std::chrono::milliseconds{ static_cast<long long>(excuteTime * 1000.0f) }, TimerEventType::TRIGGER_DEAD);
}

void Trigger::Update(const float deltaTime) { }

void Trigger::LateUpdate(const float deltaTime) { }

void Trigger::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    mInTriggerObjects.insert(opponent->GetId());
}

void Trigger::OnCollisionTerrain(const float height) { }

void Trigger::DispatchGameEvent(GameEvent* event) { }
