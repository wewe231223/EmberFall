#include "pch.h"
#include "GameEventManager.h"
#include "GameObject.h"
#include "ServerGameScene.h"

GameEventManager::GameEventManager() { }

GameEventManager::~GameEventManager() { }

void GameEventManager::SetCurrentGameScene(std::shared_ptr<class IServerGameScene> gameScene) {
    mCurrentScene = gameScene;
}

void GameEventManager::PushEvent(std::shared_ptr<GameEvent> event) {
    mEvents.emplace(event);
}

void GameEventManager::Update() {
    while (not mEvents.empty()) {        
        decltype(auto) event = mEvents.front();

        auto receiverObject = mCurrentScene->GetObjectFromId(event->receiver);
        receiverObject->DispatchGameEvent(event.get());

        mEvents.pop();
    }
}
