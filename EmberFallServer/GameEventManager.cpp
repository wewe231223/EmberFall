#include "pch.h"
#include "GameEventManager.h"
#include "GameObject.h"

GameEventManager::GameEventManager() { }

GameEventManager::~GameEventManager() { }

void GameEventManager::PushEvent(std::shared_ptr<GameEvent> event) {
    mEvents.emplace(event);
}

void GameEventManager::AddListener(std::shared_ptr<GameObject> listener) {
    mObjects.push_back(listener);
}

void GameEventManager::RemoveListener(std::shared_ptr<GameObject> listener) {
    auto it = std::find(mObjects.begin(), mObjects.end(), listener);
    std::swap(mObjects.back(), *it);
    mObjects.pop_back();
}

void GameEventManager::Update() {
    while (not mEvents.empty()) {
        decltype(auto) event = mEvents.front();

        for (auto& object : mObjects) {
            object->DispatchGameEvent(event.get());
        }

        mEvents.pop();
    }
}

void GameEventManager::EventLog(GameEvent* event) {
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Handle Event {}", typeid(static_cast<AttackEvent*>(event)).name());
}
