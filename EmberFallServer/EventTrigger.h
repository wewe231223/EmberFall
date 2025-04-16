#pragma once

#include "Trigger.h"

class EventTrigger : public Trigger {
public:
    EventTrigger(std::shared_ptr<GameObject> owner, std::shared_ptr<GameEvent> event,
        float lifeTime, float eventDelay, int32_t eventCount);
    virtual ~EventTrigger();

public:
    virtual void Init() override;
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    float mProduceEventDelay{ };
    int32_t mProduceEventCount{ };

    std::shared_ptr<GameEvent> mEvent{ };
    std::unordered_map<NetworkObjectIdType, std::pair<float, int32_t>> mProducedEventCounter{ };
};

