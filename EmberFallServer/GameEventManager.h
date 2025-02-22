#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameEventManager.h
// 
// 2025 - 02 - 10 : 게임 이벤트를 관리할 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Events.h"

class GameObject;

class GameEventManager {
public:
    GameEventManager();
    ~GameEventManager();

    GameEventManager(const GameEventManager&) = delete;
    GameEventManager(GameEventManager&&) noexcept = delete;
    GameEventManager& operator=(const GameEventManager&) = delete;
    GameEventManager& operator=(GameEventManager&&) noexcept = delete;

public:
    void PushEvent(std::shared_ptr<GameEvent> event);
    void AddListener(std::shared_ptr<GameObject> listner);
    void RemoveListener(std::shared_ptr<GameObject> listener);

    void Update();

    void EventLog(GameEvent* event);

private:
    std::queue<std::shared_ptr<GameEvent>> mEvents{ };
    std::vector<std::shared_ptr<GameObject>> mObjects{ };
};