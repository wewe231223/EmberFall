#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameEventFactory.h
// 
// 2025 - 03 - 19 : Trigger 클래스에서 포인터인 이벤트를 돌려쓰다 보니 하나의 오브젝트에만 이벤트가 전달되는 현상 발생
//                  이벤트를 복사/생성 해주는 Factory 클래스를 여기에 작성함.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Events.h"

class GameEventFactory {
public:
    template <typename EventType, typename... EventArgs>
    static std::shared_ptr<GameEvent> GetEvent(EventArgs&&... args) {
        return std::make_shared<EventType>(EventType{ args... });
    }

    static std::shared_ptr<GameEvent> CloneEvent(std::shared_ptr<GameEvent> event);
};

