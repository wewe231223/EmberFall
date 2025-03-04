#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameEventManager.h
// 
// 2025 - 02 - 21 : 게임 이벤트를 관리할 클래스
// 
//        03 - 02 : 모든 이벤트마다 게임 오브젝트를 순회하는 건 너무 비효율적이다.
//                  이벤트에 Sender, Receiver를 두어 한번에 접근할 수 있게 하자.
// 
//                  그러려면 Scene에 있는 GameObject 리스트에 접근가능 해야한다.
//                  일단은 CurrentScene을 참조하도록 하자
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
    void SetCurrentGameScene(std::shared_ptr<class IServerGameScene> gameScene);
    void PushEvent(std::shared_ptr<GameEvent> event);

    void Update();

private:
    std::queue<std::shared_ptr<GameEvent>> mEvents{ };
    std::shared_ptr<class IServerGameScene> mCurrentScene{ };
};