#include "pch.h"
#include "Trigger.h"
#include "Collider.h"
#include "GameEventManager.h"
#include "GameTimer.h"
#include "GameEventFactory.h"

Trigger::Trigger(std::shared_ptr<GameObject> owner, float lifeTime)
    : Script{ owner, ObjectTag::TRIGGER }, mLifeTime{ lifeTime } {  }

Trigger::~Trigger() { }

std::unordered_set<NetworkObjectIdType>& Trigger::GetObjects() {
    return mInTriggerObjects;
}

void Trigger::Init() {
    if (MathUtil::IsMax(mLifeTime)) {
        return;
    }

    StaticTimer::PushTimerEvent([=]() { GetOwner()->SetActive(false); gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Trigger Die"); }, mLifeTime, 1);
}

void Trigger::Update(const float deltaTime) { }

void Trigger::LateUpdate(const float deltaTime) { }

void Trigger::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto opponentId = opponent->GetId();
    if (mInTriggerObjects.contains(opponentId)) {
        return;
    }

    mInTriggerObjects.insert(opponentId);
}

void Trigger::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void Trigger::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse){ 
    auto opponentId = opponent->GetId();
    if (not mInTriggerObjects.contains(opponentId)) {
        return;
    }

    mInTriggerObjects.erase(opponentId);
}

void Trigger::OnCollisionTerrain(const float height) { }

void Trigger::DispatchGameEvent(GameEvent* event) { }
