#include "pch.h"
#include "Trigger.h"
#include "Collider.h"
#include "GameEventManager.h"
#include "GameTimer.h"
#include "GameEventFactory.h"

Trigger::Trigger(std::shared_ptr<GameObject> owner, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount)
    : Script{ owner, ObjectTag::TRIGGER }, mEvent{ event }, mLifeTime{ lifeTime },
    mProduceEventDelay{ eventDelay }, mProduceEventCount{ eventCount } {
    if (0 == mProduceEventCount) {
        mProduceEventCount = static_cast<int32_t>(mLifeTime / mProduceEventDelay);
    }
}

Trigger::~Trigger() { }

void Trigger::Init() {
    StaticTimer::PushTimerEvent([=]() { GetOwner()->SetActive(false); gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Trigger Die"); }, mLifeTime, 1);
}

void Trigger::Update(const float deltaTime) { }

void Trigger::LateUpdate(const float deltaTime) { }

void Trigger::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto opponentId = opponent->GetId();
    if (not mProducedEventCounter.contains(opponentId)) {
        mProducedEventCounter[opponentId] = std::make_pair(0.0f, 0);
    }
}

void Trigger::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
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

void Trigger::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse){ }

void Trigger::DispatchGameEvent(GameEvent* event) { }
