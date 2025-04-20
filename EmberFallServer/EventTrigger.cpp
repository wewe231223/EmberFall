#include "pch.h"
#include "EventTrigger.h"
#include "GameObject.h"
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

void EventTrigger::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (nullptr == opponent and false == GetOwner()->mSpec.active) {
        return;
    }

    auto opponentId = opponent->GetId();
    if (not mProducedEventCounter.contains(opponentId) and opponentId != mEvent->sender) {
        mProducedEventCounter.try_emplace(opponentId, 0.0f, 1);
        mEvent->receiver = opponentId;
        opponent->DispatchGameEvent(GameEventFactory::CloneEvent(mEvent));
        return;
    }

    auto& [count, delay] = mProducedEventCounter[opponentId];
    delay += GetOwner()->GetDeltaTime();
    if (delay >= mProduceEventDelay and count < mProduceEventCount) {
        count += 1;
        opponent->DispatchGameEvent(GameEventFactory::CloneEvent(mEvent));
    }
}

void EventTrigger::OnCollisionTerrain(const float height) { }

void EventTrigger::DispatchGameEvent(GameEvent* event) { }
