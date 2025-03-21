#include "pch.h"
#include "GameEventFactory.h"

std::shared_ptr<GameEvent> GameEventFactory::CloneEvent(std::shared_ptr<GameEvent> event) {
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        {
            auto attackEvent = std::make_shared<AttackEvent>();
            std::memcpy(attackEvent.get(), event.get(), sizeof(AttackEvent));
            return attackEvent;
        }

    default:
        return nullptr;
    }
}
