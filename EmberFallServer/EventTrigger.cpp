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
    auto pos = GetOwner()->GetPosition();
}

EventTrigger::~EventTrigger() { }

void EventTrigger::Init() { 
    Trigger::Init();
}

void EventTrigger::Update(const float deltaTime) { 
    auto col = std::static_pointer_cast<OBBCollider>(GetOwner()->GetBoundingObject());
    auto colpos = col->GetBoundingBox().Center;
    auto colex = col->GetBoundingBox().Extents;
}

void EventTrigger::LateUpdate(const float deltaTime) { }

void EventTrigger::OnCollisionTerrain(const float height) { }

void EventTrigger::DispatchGameEvent(GameEvent* event) { }
