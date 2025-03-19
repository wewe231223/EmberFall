#pragma once

#include "Events.h"

class GameEventFactory {
public:
    template <typename EventType, typename... EventArgs>
    static std::shared_ptr<GameEvent> GetEvent(EventArgs&&... args) {
        return std::make_shared<EventType>(args...);
    }

    static std::shared_ptr<GameEvent> EventClone(std::shared_ptr<GameEvent> event);
};

