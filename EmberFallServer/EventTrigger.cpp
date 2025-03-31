#include "pch.h"
#include "EventTrigger.h"
#include "GameTimer.h"

#include "GameEventManager.h"

EventTrigger::EventTrigger(std::shared_ptr<GameObject> owner, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount)
    : Trigger{ owner, lifeTime }, mEvent{ event }, mProduceEventDelay{ eventDelay }, mProduceEventCount{ eventCount } {
    if (0 == mProduceEventCount) {
        mProduceEventCount = static_cast<int32_t>(mLifeTime / mProduceEventDelay);
    }
}

EventTrigger::~EventTrigger() { }

void EventTrigger::Init() { 
    Trigger::Init();
}

void EventTrigger::Update(const float deltaTime) { }

void EventTrigger::LateUpdate(const float deltaTime) { }

void EventTrigger::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto opponentId = opponent->GetId();
    if (mProducedEventCounter.contains(opponentId)) {
        return;
    }

    mProducedEventCounter[opponentId] = std::make_pair(0.0f, 0);
}

void EventTrigger::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (not GetOwner()->IsActive()) {
        return;
    }

    auto opponentId = opponent->GetId();
    auto& [delayCounter, eventCount] = mProducedEventCounter[opponentId];
    if (eventCount >= mProduceEventCount) {
        return;
    }

    delayCounter += StaticTimer::GetDeltaTime();
    if (mProduceEventDelay < delayCounter) {
        auto cloneEvent = GameEventFactory::CloneEvent(mEvent);
        cloneEvent->receiver = opponentId;
        gEventManager->PushEvent(cloneEvent);

        delayCounter = 0.0f;
        ++eventCount;
    }
}

void EventTrigger::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto opponentId = opponent->GetId();
    if (not mProducedEventCounter.contains(opponentId)) {
        return;
    }

    mProducedEventCounter.erase(opponentId);
}

void EventTrigger::OnCollisionTerrain(const float height) { }

void EventTrigger::DispatchGameEvent(GameEvent* event) { }
